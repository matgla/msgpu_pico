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
#include <pico/time.h>

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
        : cursor_(0, 0)
        , foreground_(Configuration::Color::white)
        , background_(Configuration::Color::black)
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
        if (row + cursor_.y >= Configuration::height || row + cursor_.y < 0) 
        {
            return;
        }
    
        if (column + cursor_.x >= Configuration::width || column + cursor_.x < 0)
        {
            return;
        }
        const msgui::Position char_position = cursor_; 
        const msgui::Position previous_position{
            .x = cursor_.x * Configuration::font::width,
            .y = cursor_.y * Configuration::font::height
        };
        cursor_.x += column;
        cursor_.y += row;
        render_font(previous_position, text_buffer_[char_position.y][char_position.x], foreground_, background_);
        render_cursor(Configuration::Color::white, Configuration::Color::black); 
    }

    void set_cursor(int row, int column)
    {
        set_cursor_row(row);
        set_cursor_column(column);
    }

    void set_cursor_column(int column)
    {
        const msgui::Position char_position = cursor_; 
        const msgui::Position previous_position{
            .x = cursor_.x * Configuration::font::width,
            .y = cursor_.y * Configuration::font::height
        };
        if (column >= 0 && column <= Configuration::width)
        {
            cursor_.x = column;
        }
        render_font(previous_position, text_buffer_[char_position.y][char_position.x], foreground_, background_);
        render_cursor(Configuration::Color::white, Configuration::Color::black); 
    }
   
    void set_cursor_row(int row)
    {
        const msgui::Position char_position = cursor_; 
        const msgui::Position previous_position{
            .x = cursor_.x * Configuration::font::width, 
            .y = cursor_.y * Configuration::font::height
        };
        if (row >= 0 && row <= Configuration::height)
        {
            cursor_.y = row;
        }
        
        render_font(previous_position, text_buffer_[char_position.y][char_position.x], foreground_, background_);
        render_cursor(Configuration::Color::white, Configuration::Color::black);
    }
    // PIXEL MANAGEMENT 
    void set_color(int foreground, int background)
    {
        set_background_color(background);
        set_foreground_color(foreground);
    }

    void set_background_color(int background)
    {
    }

    void set_foreground_color(int foreground)
    {

    }

    void write(char c)
    {
        write(cursor_, c);   
    }

    void write(const msgui::Position cursor, char c)
    {
        if (cursor.y >= Configuration::height || cursor.x >= Configuration::width)
        {
            return;
        }

        text_buffer_[cursor.y][cursor.x] = c;
        cursor_.x = cursor.x + 1;
        cursor_.y = cursor.y;

        if (cursor_.x >= Configuration::width)
        {
            cursor_.x = 0;
            ++cursor_.y;
        }

        if (cursor_.y >= Configuration::height)
        {
            cursor_.y = 0;
        }

        const msgui::Position cursor_pos {
            .x = cursor.x * Configuration::font::width, 
            .y = cursor.y * Configuration::font::height
        };
        render_font(cursor_pos, c, foreground_, background_);
        //add_character_to_render(cursor);
    }
    // BUFFER MANAGEMENT
    
    void add_character_to_render(const msgui::Position& pos)
    {
        delta_[pos.y][pos.x] = 1;
    }

    std::span<uint16_t> get_line(const std::size_t line) const
    {
        return std::span<uint16_t>();
    }

    void __time_critical_func(copy_line_to_buffer)(std::size_t line_number, std::size_t next_line_id)
    {
        if (line_number < Configuration::resolution_height)
        {
            const auto& line = buffer_[line_number];
            auto& buffer_line = current_line_[next_line_id];
            for (int i = 0; i < line.size(); ++i)
            {
                const typename BufferType::PixelType pixel_id = line[i];
                const uint8_t colors = line.get(i);
                buffer_line[i] = Configuration::color_pallete[colors & 0xf];
                buffer_line[++i] = Configuration::color_pallete[colors >> 4]; 
                //buffer_line[++i] = Configuration::color_pallete[colors];
            }

        }
 
    }

    std::size_t __time_critical_func(fill_scanline)(std::span<uint32_t> line, std::size_t line_number)
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
    void render_cursor(Configuration::Color fg, Configuration::Color bg)
    {
        const msgui::Position cursor_pos {
            .x = cursor_.x * Configuration::font::width,
            .y = cursor_.y * Configuration::font::height
        }; 
        render_font(cursor_pos, text_buffer_[cursor_.y][cursor_.x], bg, fg);
    }
    void __time_critical_func(render_font)(const msgui::Position& position, char c,const Configuration::Color fg, const Configuration::Color bg)
    {
        constexpr int height = Configuration::font::height;
        constexpr int width = Configuration::font::width;
        const auto& bitmap = Configuration::font::data.get(c); 
        const auto& data = bitmap.data();
        
        for (int y = 0; y < height; ++y)
        {
            const uint8_t line = data[y];
            for (int x = 0; x < width; ++x)
            {
                if (line & (1 <<(8 - x)))
                {
                    buffer_[position.y + y][position.x + x] = static_cast<uint8_t>(fg);
                }
                else 
                {
                    buffer_[position.y + y][position.x + x] = static_cast<uint8_t>(bg);
                }
            }
        }
         
    }

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

    void __time_critical_func(render_screen)()
    {
    }
    void __time_critical_func(render)()
    {
   //static int i = 0; 
        //static int x = 0;
        //static bool is_rising = 0;
        
        ////test_in_ram_func();
        //if (is_rising)
        //{

        //    if (x < 640)  
        //    {
        //        for (int i = 0; i < 4; ++i)
        //        {
        //        ++x;
        //        for (int y = 0; y < 16; ++y)
        //        {
        //            buffer_[y][x] = 0; 
        //        }
        //        for (int y = 16; y < 32; ++y)
        //        {
        //            buffer_[y][x] = 1; 
        //        }
        //        }
        //    }
        //    else
        //    {
        //        is_rising = false; 
        //    }
        //}
        //if (!is_rising)
        //{

        //    if (x > 0)  
        //    {
        //        --x;
        //        for (int y = 0; y < 16; ++y)
        //        {
        //            buffer_[y][x] = 1; 
        //        }
        //        for (int y = 16; y < 32; ++y)
        //        {
        //            buffer_[y][x] = 2; 
        //        }

        //    }
        //    else
        //    {
        //        is_rising = true; 
        //    }
        //}

        ++time_to_toggle_; 
        if (time_to_toggle_ == 30)
        {
            time_to_toggle_ = 0; 
            if (cursor_color_ == Configuration::Color::white)
            {
                cursor_color_ = Configuration::Color::black; 
                cursor_bg_ = Configuration::Color::white;
            }
            else 
            {
                cursor_color_ = Configuration::Color::white;
                cursor_bg_ = Configuration::Color::black;
            }
            render_cursor(cursor_color_, cursor_bg_);
        }
        copy_line_to_buffer(0, 0);
    }
    

    void __time_critical_func(render_test_pattern)(int i)
    {
        for (int y = 8 + i * 8 + 2* i; y < (i + 2) * 8 + 2* i; ++y)
        {
            auto& line  = buffer_[y];
            for (int x = 16; x < 640-16; ++x)
            {
                line[x] = i;
            }
        }
    }

private:
    char text_buffer_[Configuration::height][Configuration::width];

    msgui::Position cursor_;

    
    using BufferType = modes::Buffer<Configuration::resolution_width, Configuration::resolution_height, Configuration::bits_per_pixel>;
    BufferType buffer_;

    using DeltaBitmap = modes::Buffer<Configuration::width, Configuration::height, 8>;
    DeltaBitmap delta_;
    constexpr static std::size_t buffer_size = 5;
    uint16_t current_line_[buffer_size][Configuration::resolution_width];
    uint16_t current_line_id_;
    uint8_t time_to_toggle_;
    Configuration::Color cursor_color_;
    Configuration::Color cursor_bg_;

    Configuration::Color foreground_;
    Configuration::Color background_;
};



} // namespace text
} // namespace modes
} // namespace vga



