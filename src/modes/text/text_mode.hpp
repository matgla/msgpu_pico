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

#include "modes/mode_base.hpp"
#include "modes/types.hpp"
#include "modes/buffer.hpp"

#include "config.hpp"

namespace vga
{
namespace modes
{
namespace text
{

template <typename Configuration, template <typename> typename Base>
class TextModeBase : public Base<Configuration>
{
public:
    using ConfigurationType = Configuration;

    TextModeBase(Vga& vga)
        : cursor_(0, 0)
        , foreground_(Configuration::Color::white)
        , background_(Configuration::Color::black)
    {
        clear();  
       // render_box();
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
        render_cursor(cursor_color_, cursor_bg_); 
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
        render_cursor(cursor_color_, cursor_bg_); 
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
        render_cursor(cursor_color_, cursor_bg_);
    }

    // PIXEL MANAGEMENT 
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
    }
    // BUFFER MANAGEMENT
    
    void clear() 
    {
        cursor_ = {0, 0};
        Base<Configuration>::clear();
        for (auto& line : text_buffer_)
        {
            line.fill(0);
        }
    }

    void render_cursor(Configuration::ColorType fg, Configuration::ColorType bg)
    {
        const msgui::Position cursor_pos {
            .x = cursor_.x * Configuration::font::width,
            .y = cursor_.y * Configuration::font::height
        }; 
        render_font(cursor_pos, text_buffer_[cursor_.y][cursor_.x], bg, fg);
    }
    void __time_critical_func(render_font)(const msgui::Position& position, char c, const Configuration::ColorType fg, const Configuration::ColorType bg)
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
                    this->framebuffer_[position.y + y][position.x + x] = fg;
                }
                else 
                {
                    this->framebuffer_[position.y + y][position.x + x] = bg;
                }
            }
        }
         
    }

    void render_box()
    {
        const auto color = Configuration::Color::white;
        for (int y = 0; y < Configuration::resolution_height; ++y)
        {
            this->framebuffer_[y][0] = color;
            this->framebuffer_[y][Configuration::resolution_width - 1] = color; 
        }

        for (int x = 0; x < Configuration::resolution_width; ++x) 
        {
            this->framebuffer_[0][x] = color; 
            this->framebuffer_[Configuration::resolution_height -1][x] = color;
        }
    }

    void __time_critical_func(render_screen)()
    {
    }

    void __time_critical_func(render)()
    {
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
        this->base_render();
    }

protected:
    std::array<std::array<char, Configuration::width>, Configuration::height> text_buffer_;

    msgui::Position cursor_;

    uint8_t time_to_toggle_;
    typename Configuration::ColorType cursor_color_;
    typename Configuration::ColorType cursor_bg_;

    typename Configuration::ColorType foreground_;
    typename Configuration::ColorType background_;
   
};

template <typename Configuration>
class PaletteTextMode : public TextModeBase<Configuration, BufferedModeBase>
{
public:
    PaletteTextMode(vga::Vga& vga) : TextModeBase<Configuration, BufferedModeBase>(vga)
    {
    }

    void set_color(int foreground, int background)
    {
        set_background_color(background);
        set_foreground_color(foreground);
    }

    void set_background_color(int background)
    {
        this->background_ = background; 
    }

    void set_foreground_color(int foreground)
    {
        this->foreground_ = foreground;
    }


    using type = Text;
};

template <typename Configuration>
class TextMode : public TextModeBase<Configuration, NonBufferedModeBase>
{
public:
    using type = Text;

    void set_color(int foreground, int background)
    {
        set_background_color(background);
        set_foreground_color(foreground);
    }

    void set_background_color(int background)
    {
        this->background_ = ((background << 8) & 0xf00) | (background & 0x0f0) | ((background) >> 8 & 0x00f); 
    }

    void set_foreground_color(int foreground)
    {
        this->foreground_ = ((foreground << 8) & 0xf00) | (foreground & 0x0f0) | ((foreground) >> 8 & 0x00f); 

    }


};

} // namespace text
} // namespace modes
} // namespace vga



