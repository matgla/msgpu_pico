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

#include <msos/dynamic_linker/dynamic_linker.hpp>
#include <msos/dynamic_linker/environment.hpp>

#include "mode/2d_graphic_mode.hpp"
#include "mode/mode_base.hpp"
#include "mode/programs.hpp"
#include "mode/types.hpp"

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
#include "messages/use_program.hpp"
#include "messages/write_buffer_data.hpp"
#include "messages/write_vertex.hpp"

#include "buffers/gpu_buffers.hpp"
#include "buffers/vertex_array_buffer.hpp"

#include "log/log.hpp"

#include "symbol_codes.h"

extern "C"
{
    struct vec3
    {
        float x;
        float y;
        float z;
    };

    struct vec4
    {
        vec4() = default;
        vec4(const vec3 &v, float w_)
            : x(v.x)
            , y(v.y)
            , z(v.z)
            , w(w_)
        {
        }

        float x;
        float y;
        float z;
        float w;
    };

    vec4 gl_Position;
    void **argument_0;
    void *argument_1;
    void *argument_2;
    void *argument_3;
    void *out_argument_0;
    vec4 gl_Color;
    vec3 arg;
}

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
        // printf("Begin primitives: %d\n", msg.type);
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

    void process(const EndPrimitives &)
    {
        // printf("End primitives\n");
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

        // printf("Got vertex: {x: %f, y: %f, z: %f\n", v.x, v.y, v.z);
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

        float v[9];
        gpu_buffers_.read(0, v, sizeof(v));

        write_offset_ += msg.size;
    }

    void process(const AllocateProgramRequest &req)
    {
        log::Log::trace("Received program allocation: %d\n", req.program_type);

        uint8_t program_id = 0;

        if (req.program_type == AllocateProgramType::AllocateProgram)
        {
            log::Log::trace("%s", "Allocate program");
            std::size_t pid = programs_.allocate_program();
            program_id      = static_cast<uint8_t>(pid);
        }
        else if (req.program_type == AllocateProgramType::AllocateFragmentShader)
        {
            log::Log::trace("%s", "Allocate fragment shader");
            std::size_t shader_id = programs_.allocate_module();
            program_id            = static_cast<uint8_t>(shader_id);
            program_type_         = ProgramType::FragmentShader;
        }
        else if (req.program_type == AllocateProgramType::AllocateVertexShader)
        {
            log::Log::trace("%s", "Allocate vertex shader");
            std::size_t shader_id = programs_.allocate_module();
            program_id            = static_cast<uint8_t>(shader_id);
            program_type_         = ProgramType::VertexShader;
        }

        log::Log::trace("Allocated pid: %d", program_id);
        AllocateProgramResponse resp{
            .program_id = program_id,
        };

        this->point_.write(resp);
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
        // log::Log::trace("Received draw arrays for id: %d, count: %d", msg.first, msg.count);

        static_cast<void>(msg);
        for (uint16_t id = 0; id < 1; ++id) // msg.first + msg.count - 1; ++id)
        {
            float v[9] = {};
            gpu_buffers_.read(id, &v, sizeof(v));

            mesh_.push_back({
                Vertex{.x = v[0], .y = v[1], .z = v[2]},
                Vertex{.x = v[3], .y = v[4], .z = v[5]},
                Vertex{.x = v[6], .y = v[7], .z = v[8]},
            });
        }
    }

    void process(const BeginProgramWrite &msg)
    {
        log::Log::trace("Received program transmission start, size: %d, pid: %d", msg.size,
                        msg.program_id);
        program_position_ = msg.program_id;
        program_data_.resize(msg.size);
        program_write_index_ = 0;
    }

    void process(const ProgramWrite &msg)
    {
        // log::Log::trace("Received program part: %d, current size: %d", msg.part,
        // program_write_index_);
        std::copy(std::begin(msg.data), std::begin(msg.data) + msg.size,
                  program_data_.begin() + program_write_index_);
        program_write_index_ += msg.size;

        if (program_write_index_ == program_data_.size())
        {
            static msos::dl::Environment env{
                msos::dl::SymbolAddress{SymbolCode::libc_printf, &printf},
            };

            static msos::dl::DynamicLinker linker;
            eul::error::error_code ec;

            *argument_0    = &arg;
            out_argument_0 = &gl_Color;

            const auto *module = linker.load_module(
                std::span<const uint8_t>(program_data_.data(), program_data_.size()),
                msos::dl::LoadingModeCopyText, env, ec);

            if (program_type_ == ProgramType::VertexShader)
            {
                programs_.add_vertex_shader(program_position_, module);
            }
            else if (program_type_ == ProgramType::FragmentShader)
            {
                programs_.add_fragment_shader(program_position_, module);
            }
        }
    }

    void process(const UseProgram &req)
    {
        programs_.use_program(req.program_id);
    }

    void process(const AttachShader &req)
    {
        programs_.assign_module(req.program_id, req.shader_id);
    }

    void render() override
    {
        this->framebuffer_.block();
        transform_mesh();
        Base::render();
        this->framebuffer_.unblock();
    }

  protected:
    constexpr uint8_t to_rgb332(float r, float g, float b)
    {
        return static_cast<uint8_t>(static_cast<uint8_t>(roundf(r * 7)) << 5 |
                                    static_cast<uint8_t>(roundf(g * 7)) << 2 |
                                    static_cast<uint8_t>(roundf(b * 3)));
    }

    void transform_mesh()
    {
        for (const auto &triangle : mesh_)
        {
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

            uint8_t color = to_rgb332(gl_Color.x, gl_Color.y, gl_Color.z);

            calculate_projection(t);

            scale(t);

            Triangle tr{.v = {vertex_2d{.x = static_cast<uint16_t>(t.vertex[0].x),
                                        .y = static_cast<uint16_t>(t.vertex[0].y)},
                              vertex_2d{.x = static_cast<uint16_t>(t.vertex[1].x),
                                        .y = static_cast<uint16_t>(t.vertex[1].y)},
                              vertex_2d{.x = static_cast<uint16_t>(t.vertex[2].x),
                                        .y = static_cast<uint16_t>(t.vertex[2].y)}}};

            for (auto &v : t.vertex)
            {
                const auto &program = programs_.get();
                for (const auto &module : program)
                {
                    if (module.type == ModuleType::VertexShader)
                    {
                        *argument_0 = &v;
                        module.module->execute();
                    }
                }
            }
            Base::add_triangle(tr, color);
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
    buffers::GpuBuffers<memory::GpuRAM> gpu_buffers_;
    buffers::VertexArrayBuffer<memory::GpuRAM, 1024> vertex_array_buffer_;

    std::vector<uint8_t> program_data_; // for now, later this can be written to static buffer
    std::size_t program_position_;
    std::size_t program_write_index_;
    enum class ProgramType
    {
        VertexShader,
        FragmentShader,
    };
    ProgramType program_type_;

    Programs programs_;
};

template <typename Configuration, typename I2CType>
using DoubleBuffered3DGraphic = GraphicMode3D<Configuration, I2CType>;

} // namespace msgpu::mode
