// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <eul/container/static_vector.hpp>
#include <eul/math/vector.hpp>

#include "mode/2d_graphic_mode.hpp"
#include "mode/mode_base.hpp"
#include "mode/programs.hpp"
#include "mode/types.hpp"
#include "mode/vertex_attribute.hpp"

#include "messages/ack.hpp"
#include "messages/allocate_program.hpp"
#include "messages/attach_shader.hpp"
#include "messages/begin_primitives.hpp"
#include "messages/begin_program_write.hpp"
#include "messages/bind.hpp"
#include "messages/draw_arrays.hpp"
#include "messages/end_primitives.hpp"
#include "messages/generate_names.hpp"
#include "messages/program_write.hpp"
#include "messages/set_vertex_attrib.hpp"
#include "messages/use_program.hpp"
#include "messages/write_buffer_data.hpp"
#include "messages/write_vertex.hpp"

#include "buffers/gpu_buffers.hpp"
#include "buffers/vertex_array_buffer.hpp"

#include "log/log.hpp"

namespace msgpu::mode
{

struct Vertex
{
    fp16 x;
    fp16 y;
    fp16 z;
};

struct FloatVertex
{
    float x;
    float y;
    float z;
};

struct FloatTriangle
{
    FloatVertex vertex[3];
};

using Vec3      = eul::math::vector<float, 3>;
using BTriangle = eul::container::static_vector<Vertex, 3>;
using Mesh      = eul::container::static_vector<BTriangle, 2048>;

struct DrawRequest
{
    uint16_t id;
    uint16_t size;
};
using DrawRequests = eul::container::static_vector<DrawRequest, 2048>;

template <typename Configuration, typename I2CType>
class GraphicMode3D : public GraphicMode2D<Configuration, I2CType>
{
  public:
    using Base = GraphicMode2D<Configuration, I2CType>;
    using Base::GraphicMode2D;
    using Base::process;
    using Base::template ModeBase<Configuration, I2CType>::process;

    GraphicMode3D(memory::VideoRam &framebuffer, memory::GpuRAM &gpuram, I2CType &i2c,
                  io::UsartPoint &point)
        : Base::GraphicMode2D(framebuffer, gpuram, i2c, point)
        , gpu_buffers_(Base::gpuram_)
        , vertex_array_buffer_(Base::gpuram_)
    {
        set_projection_matrix(90.0f, 1.0f, 1000.0f, 1.0f);
    }

    void clear() override
    {
        this->framebuffer_.block();
        Base::clear();
        mesh_.clear();
        this->framebuffer_.unblock();
    }

    void process(const BeginPrimitives &msg)
    {
        static_cast<void>(msg);
        if (mesh_.size() == mesh_.max_size())
        {
            log::Log::error("%s", "Mesh buffer is full, dropping primitive");
            return;
        }
        mesh_.push_back({});
    }

    void process(const BindObject &req)
    {
        log::Log::trace("Received binding { type: %d, target: %d, object_id: %d}", req.type,
                        req.target, req.object_id);

        if (req.type == BindObjectType::BindVertexArray)
        {
            current_buffer_ = req.object_id - 1;
        }
        if (req.type == BindObjectType::BindBuffer)
        {
            current_array_buffer_ = req.object_id - 1;
        }
    }

    void process(const SetVertexAttrib &msg)
    {
        log::Log::trace("Received set vertex attribute message");

        if (msg.index >= shader_in_arguments_size)
        {
            log::Log::error("Index outside range: %d, max %d", msg.index, shader_in_arguments_size);
            return;
        }

        auto &attrib      = vertex_attributes_[msg.index];
        attrib.normalized = msg.normalized;
        attrib.offset     = msg.pointer;
        attrib.size       = msg.size;
        attrib.stride     = msg.stride;
        attrib.type       = msg.type;
    }

    void process(const EndPrimitives &)
    {
    }

    void process(const WriteVertex &v)
    {
        if (mesh_.back().size() == mesh_.back().max_size())
        {
            if (mesh_.size() == mesh_.max_size())
            {
                return;
            }
            mesh_.push_back({});
        }

        mesh_.back().push_back({v.x, v.y, v.z});
    }

    void process(const PrepareForData &req)
    {
        write_buffer_ = req.named ? req.object_id - 1 : current_buffer_;
        log::Log::trace("Received write buffer preparation for: %d, size: %d", write_buffer_,
                        req.size);

        gpu_buffers_.allocate_memory(write_buffer_, req.size);
        write_offset_ = 0;
    }

    void process(const WriteBufferData &msg)
    {
        log::Log::trace("Received data part %d with size %d\n", msg.part, msg.size);

        gpu_buffers_.write(write_buffer_, msg.data, msg.size, write_offset_);

        write_offset_ += msg.size;
    }

    void set_projection_matrix(float view_angle, float aspect, float z_far, float z_near)
    {
        const float theta = view_angle * 0.5f;
        const float F     = 1.0f / (tanf(theta / 180.0f * 3.14f));
        const float a     = -1.0f * aspect;
        const float q     = z_far / (z_far - z_near);

        projection_ = {
            {a * F, 0, 0, 0},
            {0, -F, 0, 0},
            {0, 0, q, 1},
            {0, 0, -1 * z_near * q, 0},
        };
    }

    void process(const SetPerspective &msg)
    {
        set_projection_matrix(msg.view_angle, msg.aspect, msg.z_far, msg.z_near);
    }

    void process(const GenerateNamesRequest &msg)
    {
        log::Log::trace("Received GenerateNamesRequest for %d elements. Type %d", msg.elements,
                        msg.type);
        GenerateNamesResponse resp;
        resp.error_code = 0;
        uint16_t ids[sizeof(resp.data)];
        switch (msg.type)
        {
        case ObjectType::VertexArray: {
            vertex_array_buffer_.allocate_names(msg.elements, ids);
        }
        break;
        case ObjectType::Buffer: {
            gpu_buffers_.allocate_names(msg.elements, ids);
        }
        break;
        }
        for (uint32_t i = 0; i < msg.elements; ++i)
        {
            resp.data[i] = ids[i] + 1;
        }

        this->point_.write(resp);
    }

    void process(const DrawArrays &msg)
    {
        requests_.emplace_back(DrawRequest{
            .id   = msg.first,
            .size = msg.count,
        });
    }

    void render() override
    {
        this->framebuffer_.block();
        transform_mesh();
        Base::render();
        this->framebuffer_.unblock();
    }

  protected:
    void transform_mesh()
    {
        for (const auto &request : requests_)
        {
            float v[9] = {};
            gpu_buffers_.read(request.id, &v, sizeof(v));

            set_arguments(&v[0]);
            if (this->used_program_ && this->used_program_->vertex_shader_)
            {
                this->used_program_->vertex_shader_->execute();
            }

            set_arguments(&v[3]);
            if (this->used_program_ && this->used_program_->vertex_shader_)
            {
                this->used_program_->vertex_shader_->execute();
            }

            set_arguments(&v[6]);
            if (this->used_program_ && this->used_program_->vertex_shader_)
            {
                this->used_program_->vertex_shader_->execute();
            }

            BTriangle triangle{
                Vertex{.x = v[0], .y = v[1], .z = v[2]},
                Vertex{.x = v[3], .y = v[4], .z = v[5]},
                Vertex{.x = v[6], .y = v[7], .z = v[8]},
            };

            FloatTriangle t = convert(triangle);

            FloatVertex normal, line1, line2;
            line1.x = t.vertex[1].x - t.vertex[0].x;
            line1.y = t.vertex[1].y - t.vertex[0].y;
            line1.z = t.vertex[1].z - t.vertex[0].z;

            line2.x = t.vertex[2].x - t.vertex[1].x;
            line2.y = t.vertex[2].y - t.vertex[1].y;
            line2.z = t.vertex[2].z - t.vertex[1].z;

            normal.x = line1.y * line2.z - line1.z * line2.y;
            normal.y = line1.z * line2.x - line1.x * line2.z;
            normal.z = line1.x * line2.y - line1.y * line2.x;

            float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            normal.x /= l;
            normal.y /= l;
            normal.z /= l;

            if (normal.x * (t.vertex[0].x - camera_.x) + normal.y * (t.vertex[0].y - camera_.y) +
                    normal.z * (t.vertex[0].z - camera_.z) >=
                0.0)
            {
                // continue;
            }

            FloatVertex light{.x = 0, .y = 0, .z = -1};
            l = sqrtf(light.x * light.x + light.y * light.y + light.z * light.z);
            light.x /= l;
            light.y /= l;
            light.z /= l;

            calculate_projection(t);

            scale(t);

            Triangle tr{.v = {vertex_2d{.x = static_cast<uint16_t>(t.vertex[0].x),
                                        .y = static_cast<uint16_t>(t.vertex[0].y)},
                              vertex_2d{.x = static_cast<uint16_t>(t.vertex[1].x),
                                        .y = static_cast<uint16_t>(t.vertex[1].y)},
                              vertex_2d{.x = static_cast<uint16_t>(t.vertex[2].x),
                                        .y = static_cast<uint16_t>(t.vertex[2].y)}}};

            Base::add_triangle(tr, 0);
        }
    }

    void set_arguments(float *data)
    {
        for (int i = 0; i < shader_in_arguments_size; ++i)
        {
            auto &attribute        = vertex_attributes_[i];
            in_argument_pointer[i] = &data[attribute.offset];
        }
    }

    void calculate_projection(FloatTriangle &t)
    {
        for (auto &v : t.vertex)
        {
            v.x     = projection_[0][0] * v.x;
            v.y     = projection_[1][1] * v.y;
            v.z     = projection_[2][2] * v.z;
            float w = projection_[3][2] * v.z;

            if (w > 0.000001f || w < -0.000001f)
            {
                v.x /= w;
                v.y /= w;
            }
        }
    }

    void scale(FloatTriangle &t)
    {
        for (auto &v : t.vertex)
        {
            v.x += 1.0f;
            v.y += 1.0f;

            v.x *= 0.5f * (Configuration::resolution_width - 1);
            v.y *= 0.5f * (Configuration::resolution_height - 1);
        }
    }

    void rotate_x(FloatTriangle &t, float theta)
    {
        const float c_theta = cosf(theta);
        const float s_theta = sinf(theta);
        for (auto &v : t.vertex)
        {
            const float y = v.y;
            const float z = v.z;

            v.y = c_theta * y - s_theta * z;
            v.z = s_theta * y + c_theta * z;
        }
    }

    void rotate_y(FloatTriangle &t, float theta)
    {
        const float c_theta = cosf(theta);
        const float s_theta = sinf(theta);
        for (auto &v : t.vertex)
        {
            const float x = v.x;
            const float z = v.z;

            v.x = c_theta * x + s_theta * z;
            v.z = c_theta * z - s_theta * x;
        }
    }

    void rotate_z(FloatTriangle &t, float theta)
    {
        const float c_theta = cosf(theta);
        const float s_theta = sinf(theta);
        for (auto &v : t.vertex)
        {
            const float x = v.x;
            const float y = v.y;

            v.x = c_theta * x - s_theta * y;
            v.y = s_theta * x + c_theta * y;
        }
    }

    FloatTriangle convert(const BTriangle &t)
    {
        return FloatTriangle{convert(t[0]), convert(t[1]), convert(t[2])};
    }

    FloatVertex convert(const Vertex &v)
    {
        return FloatVertex{.x = v.x, .y = v.y, .z = v.z};
    }

    using Matrix_4x4 = eul::math::matrix<float, 4, 4>;

    uint16_t current_buffer_;
    uint16_t current_array_buffer_;
    uint16_t write_buffer_;
    std::size_t write_offset_;
    buffers::IdGenerator<1024> array_names_;
    FloatVertex camera_{.x = 0, .y = 0, .z = 0};
    Matrix_4x4 projection_;

    Mesh mesh_;
    DrawRequests requests_;
    buffers::GpuBuffers<memory::GpuRAM> gpu_buffers_;
    buffers::VertexArrayBuffer<memory::GpuRAM, 1024> vertex_array_buffer_;
    VertexAttribute vertex_attributes_[shader_in_arguments_size];
};

template <typename Configuration, typename I2CType>
using DoubleBuffered3DGraphic = GraphicMode3D<Configuration, I2CType>;

} // namespace msgpu::mode
