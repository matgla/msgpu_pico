// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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

#include <algorithm>
#include <span>

#include <cstring>

#include <msgui/Position.hpp>

#include "generator/vga.hpp"

#include "modes/types.hpp"
#include "modes/buffer.hpp"

#include <pico/sync.h>

namespace vga
{
namespace modes
{
namespace text
{

template <typename Configuration>
class TextMode 
{
public:
    using type = Text;

    TextMode(Vga& vga)
    {
        buffer_.clear();
        vga.change_mode(Configuration::mode);
        vga.setup();
        render_box();
    }

    constexpr std::size_t get_height()
    {
        return Configuration::height;
    }

    constexpr std::size_t get_width()
    {
        return Configuration::width;
    }

    // CURSOR 
    void move_cursor(int row, int column)
    {
    }

    void set_cursor(int row, int column)
    {

    }

    void set_cursor_column(int column)
    {

    }
   
    void set_cursor_row(int row)
    {

    }
    // PIXEL MANAGEMENT 
    void set_color(int foreground, int background)
    {
    }

    void set_background_color(int background)
    {
    }

    void set_foreground_color(int foreground)
    {

    }

    void write(char c)
    {

    }

    // BUFFER MANAGEMENT
    std::span<uint16_t> get_line(const std::size_t line) const
    {
        return std::span<uint16_t>();
    }

    void copy_line_to_buffer(std::size_t line_number, std::size_t next_line_id)
    {
   //     if(line_number % 3 == 0)
   //     {
   //         line_number = 226;
   //     }
   //     else if (line_number % 3 == 1)
   //     {
   //         line_number = 227;
   //     }
   //     else 
   //     {
   //         line_number = 228;
   //     }

        if (line_number < Configuration::resolution_height)
        {
            std::transform(std::begin(buffer_[line_number]), std::end(buffer_[line_number]), std::begin(current_line_[next_line_id]), [line_number](const BufferType::PixelType pixel) -> uint16_t
            {
                typename BufferType::PixelType pixel_id = pixel;
                return Configuration::color_pallete[pixel_id];
            });

        }
        else 
        {
            std::memset(current_line_[next_line_id], 0x00, sizeof(current_line_[next_line_id])); 
        }
 
    }

    std::size_t fill_scanline(std::span<uint32_t> line, std::size_t line_number)
    {
        const uint16_t* current_line = current_line_[line_number % buffer_size];
        std::size_t next_line_id = (line_number + 1) % buffer_size; 

        copy_line_to_buffer(line_number + 1, next_line_id);  
        static uint32_t postamble[] = {
            0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
        };

        line[0] = 4;
        line[1] = host_safe_hw_ptr(line.data() + 8);
        line[2] = (Configuration::resolution_width - 4) / 2;
        line[3] = host_safe_hw_ptr(current_line + 4);
        line[4] = count_of(postamble);
        line[5] = host_safe_hw_ptr(postamble);
        line[6] = 0;
        line[7] = 0;

        line[8] = (current_line[0] << 16u) | COMPOSABLE_RAW_RUN;
        line[9] = (current_line[1] << 16u) | 0;
        line[10] = (COMPOSABLE_RAW_RUN << 16u) | current_line[2]; 
        line[11] = ((Configuration::resolution_width - 5) << 16u) | current_line[3];
        return 8;
    }

    // RENDERING
   
    void render_box()
    {
        const uint16_t color = 0xf;
        for (int y = 0; y < Configuration::resolution_height; ++y)
        {
            buffer_[y][0] = color;
            buffer_[y][Configuration::resolution_width - 1] = color; 
        }

        for (int x = 0; x < Configuration::resolution_width; ++x) 
        {
            buffer_[0][x] = color; 
            buffer_[Configuration::resolution_height -1][x] = color;
        }
    }

    void render()
    {
        render_test_pattern();
        copy_line_to_buffer(0, 0);
    }

    void render_test_pattern()
    {
        for (int i = 0; i < 16; i++)
        {
            for (int x = 16; x < 64-16; ++x)
            {
                for (int y = 8 + i * 8 + 2* i; y < (i + 2) * 8 + 2* i; ++y)
                {
                    buffer_[y][x] = i;
                }
            }

        }
    }

private:
    char text_buffer_[Configuration::height][Configuration::width];
    
    using BufferType = modes::Buffer<Configuration::resolution_width, Configuration::resolution_height, Configuration::bits_per_pixel>;
    BufferType buffer_;
 
    constexpr static std::size_t buffer_size = 4;
    uint16_t current_line_[buffer_size][Configuration::resolution_width];
    uint16_t current_line_id_;
};

} // namespace text
} // namespace modes
} // namespace vga



