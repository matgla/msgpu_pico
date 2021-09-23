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

#include <cmath>

#include <eul/container/static_deque.hpp>

#include "mode/mode_base.hpp"
#include "mode/vertex.hpp"

namespace msgpu::mode
{

struct prepared_triangle
{
    float dx1;
    float dx2;
    float dx3;
    float sx;
    float ex;
    uint16_t min_y;
    uint16_t mid_y;
    uint16_t max_y;
    uint16_t color;
};

struct Triangle
{
    vertex_2d v[3];
};

template <typename Configuration, typename I2CType>
class GraphicMode2D : public ModeBase<Configuration, I2CType>
{
  public:
    using Base = ModeBase<Configuration, I2CType>;
    using Base::ModeBase;
    using Base::process;

    void add_triangle(Triangle t, uint16_t color)
    {
        sort_triangle(t);
        // printf("Adding triangle: {x: %d, y: %d}, {x: %d, y: %d}, {x: %d, y: %d}\n", t.v[0].x,
        // t.v[0].y, t.v[1].x, t.v[1].y, t.v[2].x, t.v[2].y);

        if (triangles_.size() == triangles_.max_size())
        {
            return;
        }
        triangles_.emplace_back();

        prepared_triangle &p = triangles_.back();
        const float dyba     = t.v[1].y - t.v[0].y;
        const float dyca     = t.v[2].y - t.v[0].y;
        const float dycb     = t.v[2].y - t.v[1].y;
        const float dxba     = t.v[1].x - t.v[0].x;
        const float dxca     = t.v[2].x - t.v[0].x;
        const float dxcb     = t.v[2].x - t.v[1].x;

        if (std::abs(dyba) > 0)
            p.dx1 = dxba / dyba;
        else
            p.dx1 = dxba;
        if (std::abs(dyca) > 0)
            p.dx2 = dxca / dyca;
        else
            p.dx2 = dxca;
        if (std::abs(dycb) > 0)
            p.dx3 = dxcb / dycb;
        else
            p.dx3 = dxcb;

        // move a little to round correctly
        p.sx = t.v[0].x + 0.0001f;
        p.ex = (t.v[0].y < t.v[1].y ? t.v[0].x : t.v[1].x) - 0.0001f;
        if (t.v[2].y < t.v[1].y)
        {
            std::swap(p.dx1, p.dx2);
        }
        p.color = color;
        p.min_y = t.v[0].y;
        p.mid_y = std::min(t.v[1].y, t.v[2].y);
        p.max_y = std::max(t.v[1].y, t.v[2].y);
    }

    void render() override
    {
        //  static int i = 0;
        //  printf("Render frame: %d\n", i++); printf("2D render: %ld\n", triangles_.size());
        for (uint16_t line = 0; line < Configuration::resolution_height; ++line)
        {
            std::memset(Base::line_buffer_.u8, this->clear_color_, sizeof(Base::line_buffer_));

            for (auto &triangle : triangles_)
            {
                draw_triangle_line(line, triangle);
            }
            Base::framebuffer_.write_line(line, Base::line_buffer_.u16);

            while (triangles_.size())
            {
                if (line > triangles_.front().max_y)
                {
                    triangles_.pop_front();
                }
                else
                {
                    break;
                }
            }
        }

        if (!triangles_.empty())
        {
            std::abort();
        }
        // printf("Render finished\n");
    }

    void clear()
    {
        triangles_.clear();
    }

  protected:
    void draw_horizontal_line(uint16_t x0, uint16_t x1, uint16_t color)
    {
        if (x0 > x1)
            std::swap(x0, x1);
        if (x0 >= Configuration::resolution_width || x1 >= Configuration::resolution_width)
        {
            return;
        }
        for (std::size_t i = x0; i <= x1; ++i)
        {
            Base::line_buffer_.u16[i] = color;
        }
    }

    void sort_triangle(Triangle &t)
    {
        std::sort(std::begin(t.v), std::end(t.v), [](const auto &a, const auto &b) {
            return (a.y < b.y) || (a.y == b.y && a.x < b.x);
        });
    }

    void draw_triangle_line(uint16_t line, prepared_triangle &triangle)
    {
        if (line < triangle.min_y || line > triangle.max_y)
        {
            return;
        }

        const float e_dx = line < triangle.mid_y ? triangle.dx1 : triangle.dx3;
        const float x0   = std::min(triangle.sx, triangle.ex);
        const float x1   = std::max(triangle.sx, triangle.ex);
        draw_horizontal_line(static_cast<uint16_t>(round(x0)), static_cast<uint16_t>(round(x1)),
                             triangle.color);
        triangle.sx += triangle.dx2;
        triangle.ex += e_dx;
    }

    void draw_triangle_lines(int line, prepared_triangle &t)
    {
        if (line < t.min_y || line > t.max_y)
        {
            return;
        }

        float s_dx = t.dx2;
        float e_dx = line < t.mid_y ? t.dx1 : t.dx3;

        float prev_sx = t.sx;
        float prev_ex = t.ex;

        if (line != t.max_y && s_dx > 1)
        {
            prev_sx += s_dx - 1.0f;
        }

        if (line != t.max_y && e_dx > 1)
        {
            prev_ex += e_dx - 1.0f;
        }

        if (line != t.max_y && s_dx < -1)
        {
            prev_sx += s_dx + 1.0f;
        }

        if (line != t.max_y && e_dx < -1)
        {
            prev_ex += e_dx + 1.0f;
        }

        if ((t.mid_y == t.max_y || t.mid_y == t.min_y) && t.mid_y == line)
        {
            draw_horizontal_line(static_cast<uint16_t>(round(t.sx)),
                                 static_cast<uint16_t>(round(t.ex)), t.color);
        }

        draw_horizontal_line(static_cast<uint16_t>(round(t.sx)),
                             static_cast<uint16_t>(round(prev_sx)), t.color);
        draw_horizontal_line(static_cast<uint16_t>(round(t.ex)),
                             static_cast<uint16_t>(round(prev_ex)), t.color);

        t.sx += s_dx;
        t.ex += e_dx;
    }

    eul::container::static_deque<prepared_triangle, 4096> triangles_;
};

} // namespace msgpu::mode

