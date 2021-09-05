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
using BTriangle = eul::container::static_vector<Vertex, 3>;
using Mesh = eul::container::static_vector<BTriangle, 2048>;

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
        Base::clear();
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
        
        printf("Got vertex: {x: %f, y: %f, z: %f\n", v.x, v.y, v.z);
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
        transform_mesh();
        GraphicMode2D<Configuration>::render();
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
            printf("Before rotate {x: %f, y: %f}, {x: %f, y: %f}, {x: %f, y: %f}\n", 
                t.vertex[0].x, t.vertex[0].y, 
                t.vertex[1].x, t.vertex[1].y, 
                t.vertex[2].x, t.vertex[2].y);
            //rotate_x(t, theta);
            printf("After rotate theta %f, {x: %f, y: %f}, {x: %f, y: %f}, {x: %f, y: %f}\n", theta,
                t.vertex[0].x, t.vertex[0].y, 
                t.vertex[1].x, t.vertex[1].y, 
                t.vertex[2].x, t.vertex[2].y);
 
            //rotate_z(t, theta);
            for (auto& v : t.vertex)
            {
                v.z += 3.0f;
            }

            // calculate_projection(t);
            printf("After projection {x: %f, y: %f}, {x: %f, y: %f}, {x: %f, y: %f}\n", 
                t.vertex[0].x, t.vertex[0].y, 
                t.vertex[1].x, t.vertex[1].y, 
                t.vertex[2].x, t.vertex[2].y);
 
            scale(t);

            printf("Adding triangle: {y: %f, x: %f}, {y: %f, x: %f}, {y: %f, x: %f}\n", t.vertex[0].y, t.vertex[0].x, t.vertex[1].y, t.vertex[1].x, t.vertex[2].y, t.vertex[2].x); 
            Triangle tr {
                .a = vertex_2d {
                    .x = static_cast<uint16_t>(t.vertex[0].x), 
                    .y = static_cast<uint16_t>(t.vertex[0].y) 
                },
                .b = vertex_2d {
                    .x = static_cast<uint16_t>(t.vertex[1].x), 
                    .y = static_cast<uint16_t>(t.vertex[1].y) 
                },
                .c = vertex_2d {
                    .x = static_cast<uint16_t>(t.vertex[2].x), 
                    .y = static_cast<uint16_t>(t.vertex[2].y) 
                } 
            };
 
            GraphicMode2D<Configuration>::add_triangle(tr, 0xfff);
        }
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

    FloatTriangle convert(const BTriangle& t)
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
};

template <typename Configuration>
using DoubleBuffered3DGraphic = GraphicMode3D<Configuration>;

} // namespace msgpu::mode
