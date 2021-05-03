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

#include "modes/mode_base.hpp"

#include "modes/types.hpp"

#include "messages/begin_primitives.hpp"

namespace vga::modes::graphic 
{

template <typename Configuration, template <typename> typename Base>
class GraphicModeBase : public Base<Configuration>
{
public: 

    void clear() 
    {
        Base<Configuration>::clear();
    }

    void render() 
    {
    }

    void set_pixel(int x, int y, uint16_t color) 
    {
        Base<Configuration>::set_pixel({.x = x, .y = y}, static_cast<typename Configuration::Color>(color));
    }

    void draw_line(int x1, int y1, int x2, int y2)
    {
        int d, dx, dy, ai, bi, xi, yi;
        int x = x1, y = y1;

        if (x1 < x2) 
        {
            xi = 1;
            dx = x2 - x1;
        }
        else 
        {
            xi = -1;
            dx = x1 - x2;
        }
    
        if (y1 < y2) 
        {
            yi = 1;
            dy = y2 - y1;
        }
        else 
        {
            yi = -1;
            dy = y1 - y2;
        }
    
        uint16_t color = 0xfff;
        set_pixel(x, y, color);
        
        if (dx > dy)
        {
            ai = (dy - dx) * 2;
            bi = dy * 2;
            d = bi - dx; 

            while (x != x2) 
            {
                if (d >= 0)
                {
                    x += xi;
                    y += yi;
                    d += ai;
                }
                else 
                {
                    d += bi;
                    x += xi;
                }
                set_pixel(x, y, color);
            }
        }
        else 
        {
            ai = (dx - dy) * 2;
            bi = dx * 2;
            d = bi - dy;
            while (y != y2)
            {
                if (d >= 0)
                {
                    x += xi;
                    y += yi;
                    d += ai;
                }
                else 
                {
                    d += bi;
                    y += yi;
                }
                set_pixel(x, y, color);
            }
        }
    }

    void start_primitives(PrimitiveType type)
    {
        primitive_type = type; 
        primitive_vertex_counter = 0;
    }

    void end_primitives()
    {
        primitive_vertex_counter = 0;
    }


    void write_vertex(float x, float y, float z)
    {
        const int expected_vertexes = get_vertex_count_for(primitive_type);
        
        vertex_buffer[primitive_vertex_counter] = {.x = x + 1, .y = y + 1, .z = z + 1};

        ++primitive_vertex_counter;
        printf("Write vertex: %f %f %f\n", x, y, z);
        if (primitive_vertex_counter >= expected_vertexes)
        {
            printf("Write primitve\n");
            draw_primitive();   
            primitive_vertex_counter = 0;
        }
    }

    struct Vertex3
    {
        float x;
        float y;
        float z;
    };
protected:

    void draw_primitive()
    {
        const int vertexes = get_vertex_count_for(primitive_type);
        for (int i = 0; i < vertexes; ++i)
        {
            set_pixel(vertex_buffer[i].x * 50, vertex_buffer[i].y * 50, 0xfff);
        }
    }

    int get_vertex_count_for(PrimitiveType type)
    {
        switch (type)
        {
            case PrimitiveType::triangle: return 3;
            case PrimitiveType::square: return 4;
            case PrimitiveType::point: return 1;
            case PrimitiveType::line: return 2;
        }

        return 0;
    }

    PrimitiveType primitive_type;
    int primitive_vertex_counter;
    std::array<Vertex3, 4> vertex_buffer;
};

template <typename Configuration>
class PaletteGraphicMode : public GraphicModeBase<Configuration, BufferedModeBase>
{

};

template <typename Configuration>
class GraphicMode : public GraphicModeBase<Configuration, NonBufferedModeBase>
{
public:
    using type = vga::modes::Graphic;
    using ConfigurationType = Configuration;
};


} // namespace vga::modes::graphic
