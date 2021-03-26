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

#include "modes/text/80x25.hpp"

#include <cstring>
#include <cstdio>

#include "modes/draw/draw_480_6bit.hpp"

#include "memory/video_ram.hpp"

#include <msgui/fonts/Font5x7.hpp>

#include "interfaces/usart.hpp"
#include "modes/colors.hpp"


namespace vga
{
namespace modes
{
namespace text
{

namespace
{
    // 76800 frame buffer
    // 2000 text buffer
    // 250 delta
    // 4000 attributes

template <typename Font, int width, int height>
struct Configuration
{
    constexpr static auto font = Font::data;

    constexpr static int character_height = Font::height + 1;
    constexpr static int character_width = Font::width + 1;

    constexpr static int resolution_width = character_width * width;
    constexpr static int resolution_height = character_height * height;

    constexpr static int video_ram_size = resolution_width * resolution_height * 0.8;
    constexpr static int text_buffer_size = width * height;
    constexpr static int attributes_size = width * height * 2;
    constexpr static int delta_bitmap_size = width * height / 8;
    constexpr static auto draw_function = &draw_480_6bit_wrapper;
};

constexpr static Configuration<msgui::fonts::Font5x7, 80, 25> configuration;

}

void Mode80x25::fill_scanline(std::span<uint16_t> line, std::size_t line_number)
{
    std::memset(line.data(), 0, line.size() * sizeof(uint16_t));
 
    std::size_t text_line = line_number / configuration.font.height(); 

    std::size_t bx = 0;
    if (text_line > get_height())
    {
        return;
    }
    for (int i = 0; i < 10; ++i)
    {
        char c = get_character(text_line, i);
        const auto& f = configuration.font.get(c);
        for (int x = 0; x < configuration.character_width - 1; ++x) 
        {
            if (f.getPixel(x, line_number % 7) == 1)
            {
                line[bx] = 0xfff; 
            }
            else
            {
                //line[bx] = 0;
            }
            ++bx;
        }
    }

}

Mode80x25::Mode80x25(vga::Vga& vga)
    : text_buffer_(reinterpret_cast<char*>(video_ram) 
        + configuration.video_ram_size)
    , changed_bitmap_(reinterpret_cast<uint8_t*>(text_buffer_) 
        + configuration.text_buffer_size)
    , attributes_(reinterpret_cast<uint16_t*>(changed_bitmap_ + configuration.delta_bitmap_size))
{
    render_test_box();

    clear_changed_bitmap();
    clear_text_buffer();
}

void Mode80x25::move_cursor(int row_offset, int column_offset)
{
    if (row_offset + cursor_row_ >= get_height() || row_offset + cursor_row_ < 0)
    {
        return;
    }

    if (column_offset + cursor_column_ >= get_width() || column_offset + cursor_column_ < 0)
    {
        return;
    }

    // rerender old position to remove cursor artifact
    add_pixel_to_render(cursor_row_, cursor_column_);

    cursor_column_ += column_offset;
    cursor_row_ += row_offset;
    draw_cursor_ = true;
}

void Mode80x25::set_cursor_row(int row)
{
    if (row >= 0 && row <= get_height())
    {
        cursor_row_ = row;
    }
}

void Mode80x25::set_cursor_column(int column)
{
    if (column >= 0 && column <= get_width())
    {
        cursor_column_ = column;
    }
}

void Mode80x25::set_cursor(int row, int column)
{
    set_cursor_row(row);
    set_cursor_column(column);
}


void Mode80x25::render_test_box()
{
    for (int i = 0; i < configuration.resolution_width; ++i)
    {
        set_pixel({.x = i, .y = 0}, 15);
        set_pixel({.x = 0, .y = i % configuration.resolution_height}, 20);
        set_pixel({.x = configuration.resolution_width - 1, .y = i % configuration.resolution_height}, 17);
        set_pixel({.x = i, .y = configuration.resolution_height - 1}, 18);
    }
}


void Mode80x25::write(uint8_t row, uint8_t column, char c)
{
    if (column >= get_width() || row >= get_height())
    {
        return;
    }

    cursor_column_ = column + 1;
    cursor_row_ = row;
    draw_cursor_ = true;

    if (cursor_column_ >= get_width())
    {
        cursor_column_ = 0;
        ++cursor_row_;
    }
    if (cursor_row_ >= get_height())
    {
        cursor_row_ = 0;
    }

    text_buffer_[row * get_width() + column] = c;

    add_pixel_to_render(row, column);
    set_attribute(row, column, foreground_ | (background_ << 6));
}

void Mode80x25::write(char c)
{
    write(cursor_row_, cursor_column_, c);
}

void Mode80x25::set_foreground_color(int foreground)
{
    foreground_ = foreground;
}

void Mode80x25::set_background_color(int background)
{
    background_ = background;
}

void Mode80x25::set_color(int foreground, int background)
{
    set_foreground_color(foreground);
    set_background_color(background);
}

void Mode80x25::render()
{
    --time_to_blink_;
    
    render_screen();
    clear_changed_bitmap();
    render_cursor();

    if (time_to_blink_ == 0)
    {
        time_to_blink_ = time_to_blink_default;
    }
}

void Mode80x25::set_pixel(msgui::Position position, int color)
{
    const int pixel_slot = position.y * configuration.resolution_width / 5 + position.x / 5; // 5 pixels in uint32_t
    constexpr int bits_per_pixel = 6;
    constexpr int pixels_in_4bytes = 5;
    const int offset = bits_per_pixel * (position.x % pixels_in_4bytes);
    video_ram[pixel_slot] &= ~(0x3f << offset);
    video_ram[pixel_slot] |= (color << offset);
}

void Mode80x25::render_font(const auto& bitmap, 
    const int row, const int column, 
    const int foreground, const int background)
{
    constexpr int height = configuration.character_height - 1;
    constexpr int width = configuration.character_width - 1;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (bitmap.getPixel(x, y))
            {
                set_pixel({.x = column + x, .y = row + y}, foreground);
            }
            else
            {
                set_pixel({.x = column + x, .y = row + y}, background);
            }
        }
        set_pixel({.x = column + width, .y = row + y}, background);
    }
    for (int x = 0; x < configuration.character_width; ++x)
    {
        set_pixel({.x = column + x, .y = row + height}, background);
    }
}

void Mode80x25::clear_text_buffer() 
{
    std::memset(text_buffer_, 0, configuration.text_buffer_size);
}


void Mode80x25::clear_changed_bitmap() 
{
    std::memset(changed_bitmap_, 0, configuration.delta_bitmap_size);
}

void Mode80x25::add_pixel_to_render(int row, int column)
{
    constexpr int pixels_per_byte = 8;
    const int char_position = row * get_width() / pixels_per_byte + column / pixels_per_byte;
    const int offset = column % pixels_per_byte;
    changed_bitmap_[char_position] &= ~(0x1 << offset);
    changed_bitmap_[char_position] |= 0x1 << offset;
}

void Mode80x25::set_attribute(int row, int column, int attribute)
{
    const int attribute_position = row * get_width() + column;
    attributes_[attribute_position] = attribute;
}

uint16_t Mode80x25::get_attribute(int row, int column) const
{
    const int attribute_position = row * get_width() + column;
    return attributes_[attribute_position];
}

void Mode80x25::render_screen()
{
    constexpr static int pixels_in_byte = 8;
    for (int y = 0; y < get_height(); ++y)
    {
        for (int x = 0; x < get_width() / pixels_in_byte; ++x)
        {
            const int char_position = y * get_width() / pixels_in_byte + x;

            if (changed_bitmap_[char_position])
            {
                for (int i = 0; i < pixels_in_byte; ++i)
                {
                    if (changed_bitmap_[char_position] & (1 << i))
                    {
                        const char c = text_buffer_[y * get_width() + (pixels_in_byte * x) + i];

                        const int attribute_position = y * get_width() + (x * pixels_in_byte + i);
                        const int foreground = attributes_[attribute_position] & 0x3f;
                        const int background = (attributes_[attribute_position] >> 6) & 0x3f;

                        render_font(configuration.font.get(c), 
                            y * configuration.character_height, 
                            x * pixels_in_byte * configuration.character_width + i * configuration.character_width, 
                            foreground, background);
                    }
                }
            }
        }
    }
}

void Mode80x25::render_cursor() 
{
    if (time_to_blink_ == 0 && !draw_cursor_)
    {
        cursor_visible_ = !cursor_visible_;
        
    }

    if (cursor_visible_) 
    {
        draw_cursor_ = true;
    }

    if (draw_cursor_)
    {
        draw_cursor_ = false;
        const char c = get_character(cursor_row_, cursor_column_);
        const auto& f = configuration.font.get(c);
        const int fg = get_attribute(cursor_row_, cursor_column_) & 0x3f;
        const int bg = (get_attribute(cursor_row_, cursor_column_) >> 6) & 0x3f;
        const int cursor_fg = colors::white - fg;
        const int cursor_bg = colors::white - bg;
        for (int y = 0; y < configuration.character_height; y++)
        {
            for (int x = 0; x < configuration.character_width; x++)
            {
                int color = cursor_bg;
                if (f.getPixel(x, y))
                {
                    color = cursor_fg;
                }
                set_pixel({.x = cursor_column_ * configuration.character_width + x, .y = cursor_row_ * configuration.character_height + y}, color);
            }
        }
    }
    else 
    {
        add_pixel_to_render(cursor_row_, cursor_column_);
    }
}

char Mode80x25::get_character(int row, int column) const
{
    return text_buffer_[row * get_width() + column];
}


} // namespace text
} // namespace modes
} // namespace vga



