// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
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

#include <eul/math/matrix.hpp>
#include <eul/math/vector.hpp>

#include "modes/mode_base.hpp"

#include "modes/types.hpp"

#include "messages/begin_primitives.hpp"

#include "board.hpp"

namespace vga::modes::graphic 
{

static inline uint32_t clear_time = 0;

template <typename Configuration, template <typename> typename Base>
class GraphicModeBase : public Base<Configuration>
{
public: 

    void clear() 
    {
        clear_time = msgpu::get_millis(); 
        Base<Configuration>::clear();
    }

    void render() 
    {
        Base<Configuration>::base_render();
    }

    void __time_critical_func(set_pixel)(int x, int y, uint16_t color) 
    {
        Base<Configuration>::set_pixel({.x = x, .y = y}, static_cast<Configuration::Color>(color));
    }

    void __time_critical_func(draw_line)(int x1, int y1, int x2, int y2, uint16_t color = Configuration::Color::white)
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
    
        //uint16_t color = Configuration::Color::white;
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
                set_pixel(x, y, color); }
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
        theta += 0.01;
        if (theta > 6.28)
        {
            theta = 0;
        }
    }

    void end_primitives()
    {
        primitive_vertex_counter = 0;
    }


    void __time_critical_func(write_vertex)(float x, float y, float z)
    {
        const int expected_vertexes = get_vertex_count_for(primitive_type);
        
        vertex_buffer[primitive_vertex_counter] = {x, y, z, 1};

        ++primitive_vertex_counter;
        if (primitive_vertex_counter >= expected_vertexes)
        {
            static uint32_t s = 0;
            draw_primitive();   
        
            const uint32_t f = msgpu::get_millis();
            //printf("Between draw %d\n", (f - s));
            s = msgpu::get_millis(); 
            //printf("P drawn and took %d ms.\n", (f - clear_time));
            primitive_vertex_counter = 0;
        }
    }

    void set_perspective(float angle, float aspect, float z_far, float z_near)
    {
        const float theta = angle / 2.0f;
        const float F = 1.0f / (tanf(theta / 180.0f * 3.14f)); 
        const float a = aspect; 
        const float q = z_far / (z_far - z_near);

        projection_ = {
            { a * F,    0,           0, 0 },
            {     0,    F,           0, 0 },
            {     0,    0,           q, 1 },
            {     0,    0, -1 * z_near * q, 0}
        };

        //printf("Projection matrix setted\n");

        //for (std::size_t i = 0; i < projection_.rows(); ++i)
        //{
        //    for (std::size_t j = 0; j < projection_.columns(); ++j)
        //    {
        //        printf("%f, ", projection_[i][j]);
        //    }
        //    printf("\n");
        //}


    }

    struct Vertex 
    {
        float x;
        float y;
        float z;
    };

    struct Triangle 
    {
        Vertex data[3];
    };

    void __time_critical_func(draw_triangle)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, bool fill, uint16_t color)
    {
        printf("Drawing triangle\n");
        Vertex triangle[3] = {
            { .x = static_cast<float>(x1), .y = static_cast<float>(y1), .z = 0 },
            { .x = static_cast<float>(x2), .y = static_cast<float>(y2), .z = 0 },
            { .x = static_cast<float>(x3), .y = static_cast<float>(y3), .z = 0 }
        };

        std::sort(std::begin(triangle), std::end(triangle), 
            [](const Vertex& a, const Vertex& b) {
                return a.y < b.y;
            }
        );
        
        const auto& A = triangle[0];
        const auto& B = triangle[1];
        const auto& C = triangle[2];

        float dx1 = 0;
        float dx2 = 0; 
        float dx3 = 0;
    
        
        if (B.y - A.y > 0) 
        {
            dx1 = (B.x - A.x) / (B.y - A.y);
        }
        if (C.y - A.y > 0) 
        {
            dx2 = (C.x - A.x) / (C.y - A.y);
        }
        if (C.y - B.y > 0) 
        {
            dx3 = (C.x - B.x) / (C.y - B.y);
        }

        printf("Draw /\\\n\n");
        for (int i = 0; i < 3; ++i)
        {
            printf ("%f: x: %f, y: %f\n", i, triangle[i].x, triangle[i].y);
        }
        printf("=================\n");
        printf(" dx1: %f\n", dx1);
        printf(" dx2: %f\n", dx2);
        printf(" dx3: %f\n", dx3);


        Vertex S = A;
        Vertex E = A;
        if (dx1 > dx2) 
        {
            for (;S.y <= B.y; S.y++, E.y++, S.x+=dx2, E.x+=dx1)
            {
                draw_line(S.x, S.y, E.x, S.y, color);
            }
            E = B; 
            for (;S.y <= C.y; S.y++, E.y++, S.x+=dx2, E.x+=dx3)
            {
                draw_line(S.x, S.y, E.x, S.y, color);
            }
        }
        else 
        {
            for (;S.y <= B.y; S.y++, E.y++, S.x+=dx1, E.x+=dx2)
            {
                draw_line(S.x, S.y, E.x, S.y, color);
            }
            S = B; 
            for (;S.y <= C.y; S.y++, E.y++, S.x+=dx3, E.x+=dx2)
            {
                draw_line(S.x, S.y, E.x, S.y, color);
            }
        }
    }


protected:
    inline static float theta = 0.0f;
    using Vector4 = eul::math::vector<float, 4>;

    void __time_critical_func(draw_primitive)()
    {
        const int vertexes = get_vertex_count_for(primitive_type);
  
        // For now only Triangle supported 

        Matrix_4x4 rotate_x = {
            {1,          0,           0, 0},
            {0, cos(theta), -sin(theta), 0},
            {0, sin(theta),  cos(theta), 0},
            {0,          0,           0, 1} 
        };

        Matrix_4x4 rotate_z = {
            { cos(theta), -sin(theta), 0, 0},
            { sin(theta), cos(theta) , 0, 0},
            {          0,          0 , 1, 0},
            {          0,          0 , 0, 1}
        };

        Triangle t;
         
        int prev_x, prev_y, first_x, first_y;
        for (int i = 0; i < vertexes; ++i)
        {
            Vector4 translated = vertex_buffer[i];
           
            translated = translated * rotate_x;
            translated = translated * rotate_z; 

            translated[2] += 3.0f;

            Vector4 cartesian_vector = translated * projection_;
    

            float x = cartesian_vector[0];
            float y = cartesian_vector[1];
            if (cartesian_vector[3] != 0)
            {
                x /= cartesian_vector[3];
                y /= cartesian_vector[3];
            }
            
            x += 1.0f;
            y += 1.0f; 


            x *= 0.5f * (Configuration::resolution_width - 1);
            y *= 0.5f * (Configuration::resolution_height - 1);
            
            t.data[i].x = x;
            t.data[i].y = y;
        }

        draw_triangle(t.data[0].x, t.data[0].y, t.data[1].x, t.data[1].y, t.data[2].x, t.data[2].y, true, 6);
    }

    int __time_critical_func(get_vertex_count_for)(PrimitiveType type)
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

    using Matrix_4x4 = eul::math::matrix<float, 4, 4>;
    
    PrimitiveType primitive_type;
    int primitive_vertex_counter;
    std::array<Vector4, 4> vertex_buffer;
    Matrix_4x4 projection_; 
};

template <typename Configuration>
class PaletteGraphicMode : public GraphicModeBase<Configuration, DoubleBufferedPaletteBase>
{
public:
    using type = vga::modes::Graphic;
    using ConfigurationType = Configuration;
};

template <typename Configuration>
class GraphicMode : public GraphicModeBase<Configuration, SingleBufferedRawBase>
{
public:
    using type = vga::modes::Graphic;
    using ConfigurationType = Configuration;
};


} // namespace vga::modes::graphic
