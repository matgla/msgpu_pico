// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once 

#include <array>
#include <algorithm>
#include <cstdio>
#include <cmath>

#include <eul/container/static_vector.hpp>
#include <eul/math/vector.hpp>

#include "mode/mode_base.hpp"
#include "mode/2d_graphic_mode.hpp"
#include "mode/types.hpp"

#include "messages/begin_primitives.hpp"
#include "messages/end_primitives.hpp"
#include "messages/write_vertex.hpp"

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

using Vec3 = eul::math::vector<float, 3>;
using Triangle = eul::container::static_vector<Vertex, 3>;
using Mesh = eul::container::static_vector<Triangle, 2048>;

struct ProcessedTriangle
{
    uint16_t sy; 
    uint16_t cy;
    uint16_t by;

    float sx;
    float ex; 
    float bx;

    float dx1;
    float dx2;
    float dx3;
};

using ProcessedBuffer = eul::container::static_vector<ProcessedTriangle, 2048>;

template <typename Configuration>
class GraphicMode3D : public GraphicMode2D<Configuration>
{
public:
    using Base = GraphicMode2D<Configuration>;
    using GraphicMode2D<Configuration>::GraphicMode2D; 
    using GraphicMode2D<Configuration>::process;
    using GraphicMode2D<Configuration>::template ModeBase<Configuration>::process;

    void clear() override
    {
        printf("Clearing mesh\n");
        mesh_.clear();
    }

    void process(const BeginPrimitives& msg)
    {
        printf("Begin primitives: %d\n", msg.type);
        if (mesh_.size() == mesh_.max_size())
        {
            printf("Mesh buffer is full, dropping primitive\n");
            return;
        }
        mesh_.push_back({});
    }

    void process(const EndPrimitives& )
    {
        printf("End primitives\n");
    }

    void process(const WriteVertex& v)
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

    void process(const SetPerspective& msg)
    {
        const float theta = msg.view_angle / 2.0f;
        const float F = 1.0f / (tanf(theta / 180.0f * 3.14f));
        const float a = msg.aspect;
        const float q = msg.z_far / (msg.z_far - msg.z_near);

        printf ("Calculated projection: %f %f %f %f\n", a*F, F, q, -1 * msg.z_near * q);

        projection_ = {
            { a * F,    0,                   0,    0 },
            {     0,    F,                   0,    0 },
            {     0,    0,                   q,    1 },
            {     0,    0, -1 * msg.z_near * q,    0 }
        };
    }

    void render() override
    {
        printf("Render\n"); 
        triangles_.clear();
        transform_mesh();

        for (uint16_t i = 0; i < Configuration::resolution_height; ++i)
        {
            std::memset(Base::line_buffer_.u8, 0, sizeof(Base::line_buffer_));
            for (auto& t : triangles_)
            {
                step_draw(t, i);
            }

            Base::framebuffer_.write_line(i, Base::line_buffer_.u16);
        }
    }

protected:
    void transform_mesh()
    {

        static float theta = 0.0f;
        theta += 0.1f;
        printf("Mesh size: %ld\n", mesh_.size());
        for (const auto& triangle : mesh_)
        {
            FloatTriangle t = convert(triangle);
            rotate_x(t, theta);
            //rotate_z(t, theta);
            for (auto& v : t.vertex)
            {
                v.z += 3.0f;
            }
            calculate_projection(t);
            scale(t);
            sort_triangle(t);
            auto i_t = interpolate_triangle(t);
            triangles_.push_back(i_t);
        }
    }

    ProcessedTriangle interpolate_triangle(FloatTriangle& t)
    {
        for (auto& v : t.vertex)
        {
            if (v.x < 0) v.x = 0;
            if (v.y < 0) v.y = 0;
        }

        const auto& v = t.vertex;
        printf("Triangle {%f %f} {%f %f} {%f %f}\n", v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y);

        ProcessedTriangle r;
        r.dx1 = 0;
        r.dx2 = 0;
        r.dx3 = 0;

        if (v[1].y - v[0].y > 0)
        {
            r.dx1 = (v[1].x - v[0].x) / (v[1].y - v[0].y);
        }
        if (v[2].y - v[0].y > 0)
        {
            r.dx2 = (v[2].x - v[0].x) / (v[2].y - v[0].y);
        }
        if (v[2].y - v[1].y > 0)
        {
            r.dx3 = (v[2].x - v[1].x) / (v[2].y - v[1].y);
        }

        r.sx = v[0].x;
        r.sy = static_cast<uint16_t>(v[0].y);
        r.ex = r.sx;
        r.cy = static_cast<uint16_t>(v[2].y);
        r.bx = v[1].x; 
        r.by = static_cast<uint16_t>(v[1].y);
        return r;
    }

    void step_draw(ProcessedTriangle& t, std::size_t line)
    {
        if (line < t.sy || line >= t.cy)
        {
            return;
        }

//        printf("Draw line: { %f %f }\n", t.sx, t.ex);
//        printf("Dx1: %f, dx2: %f, dx3: %f\n", t.dx1, t.dx2, t.dx3);
        Base::draw_horizontal_line(static_cast<uint16_t>(t.sx), static_cast<uint16_t>(t.ex), 0xfff);

        if (t.dx1 > t.dx2)
        {
            if (t.sy < t.by)
            {
                t.ex += t.dx1;
            }
            else if (t.sy == t.by)
            {
                t.ex = t.bx;
            }
            else if (t.sy <= t.cy)
            {
                t.ex += t.dx3;
            }
            t.sx += t.dx2;

        }
        else 
        {
            if (t.sy < t.by)
            {
                t.sx += t.dx1;
            }
            else if (t.sy == t.by)
            {
                t.sx = t.bx;
                t.sy = t.sy;
            }
            else if (t.sy <= t.cy)
            {
                t.sx += t.dx3;
            }
            t.ex += t.dx2;
        }
        ++t.sy;
    }

    void sort_triangle(FloatTriangle& t)
    {
        auto& v = t.vertex;
        if (v[1].y < v[0].y) std::swap(v[1], v[2]);
        if (v[2].y < v[0].y) std::swap(v[2], v[0]);
        if (v[2].y < v[1].y) std::swap(v[2], v[1]);
    }
    void calculate_projection(FloatTriangle& t)
    {
        for (auto& v : t.vertex)
        {
            printf("Projection %f %f %f %f\n", projection_[0][0], projection_[1][1], projection_[2][2], projection_[3][2]);
            v.x = projection_[0][0] * v.x;
            v.y = projection_[1][1] * v.y;
            v.z = projection_[2][2] * v.z + 1;
            float w = projection_[3][2] * v.z;

            if (w > 0.000001f || w < -0.000001f)
            {
                v.x /= w;
                v.y /= w;
            }

       }
    }

    void scale(FloatTriangle& t)
    {
        for (auto& v : t.vertex)
        {
            v.x += 1.0f;
            v.y += 1.0f;
            if (v.x > 2.0f || v.x < 0.0f)
            {
                printf("X smash detected: %f\n", v.x);
            }
            if (v.y > 2.0f || v.y < 0.0f)
            {
                printf("Y smash detected: %f\n", v.y);
            }

            v.x *= 0.5f * (Configuration::resolution_width - 1);
            v.y *= 0.5f * (Configuration::resolution_height - 1);
        } 
    }

    void rotate_x(FloatTriangle& t, float theta)
    {
        const float c_theta = static_cast<float>(cos(theta));
        const float s_theta = static_cast<float>(sin(theta));
        for (auto& v : t.vertex)
        {
            v.y = c_theta * v.y - s_theta * v.z;
            v.z = s_theta * v.y + c_theta * v.z;
        }
    }

    void rotate_y(FloatTriangle& t, float theta)
    {
        const float c_theta = static_cast<float>(cos(theta));
        const float s_theta = static_cast<float>(sin(theta));
        for (auto& v : t.vertex)
        {
            v.x = c_theta * v.x + s_theta * v.z;
            v.z = c_theta * v.z - s_theta * v.x;
        }
    }

    void rotate_z(FloatTriangle& t, float theta)
    {
        const float c_theta = static_cast<float>(cos(theta));
        const float s_theta = static_cast<float>(sin(theta));
        for (auto& v : t.vertex)
        {
            v.x = c_theta * v.x - s_theta * v.y;
            v.y = s_theta * v.x + c_theta * v.y;
        }
    }

    FloatTriangle convert(const Triangle& t)
    {
        return FloatTriangle {
            convert(t[0]),
            convert(t[1]),
            convert(t[2])
        };
    }

    FloatVertex convert(const Vertex& v)
    {
        return FloatVertex {.x = v.x, .y = v.y, .z = v.z };
    }

    using Matrix_4x4 = eul::math::matrix<float, 4, 4>;

    Matrix_4x4 projection_;
    Mesh mesh_;
    // triangles to be rendred on 2D plane
    ProcessedBuffer triangles_;
};

template <typename Configuration>
using DoubleBuffered3DGraphic = GraphicMode3D<Configuration>;

} // namespace msgpu::mode
