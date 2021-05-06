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

#include "modes/modes.hpp"

#include <any>

#include "generator/vga.hpp"
#include "board.hpp"

namespace vga
{

Mode::Mode()
{
    mode_.emplace<Graphic_320x240_12bit>();
}

void Mode::switch_to(const vga::modes::Modes mode)
{
    clear();
    switch(mode)
    {
        case modes::Modes::Text_80x30_16:
        {
            msgpu::set_resolution(Text_40x30_16_8x16::ConfigurationType::resolution_width, Text_40x30_16_8x16::ConfigurationType::resolution_height);
            mode_.emplace<Text_80x30_16_8x16>(vga::get_vga());
        } break;
        case modes::Modes::Text_40x30_16:
        {
            msgpu::set_resolution(Text_40x30_16_8x16::ConfigurationType::resolution_width, Text_40x30_16_8x16::ConfigurationType::resolution_height);
            mode_.emplace<Text_40x30_16_8x16>(vga::get_vga());
        } break;
        case modes::Modes::Text_40x30_12bit:
        {
            msgpu::set_resolution(Text_40x30_12bit_8x16::ConfigurationType::resolution_width, Text_40x30_12bit_8x16::ConfigurationType::resolution_height);
            mode_.emplace<Text_40x30_12bit_8x16>(vga::get_vga());
        } break;
        case modes::Modes::Graphic_320x240_12bit:
        {
            msgpu::set_resolution(Graphic_320x240_12bit::ConfigurationType::resolution_width, Graphic_320x240_12bit::ConfigurationType::resolution_height);
            mode_.emplace<Graphic_320x240_12bit>();
        } break;
    }
}

void Mode::render()
{
   // test_mode_.render();
    std::visit([](auto&& mode) {
        mode.render();
    }, mode_);
}

std::size_t Mode::fill_scanline(std::span<uint32_t> line, std::size_t line_number)
{
//    return test_mode_.fill_scanline(line, line_number);
    return std::visit([line, line_number](auto&& mode) ->std::size_t {
        return mode.fill_scanline(line, line_number);
    }, mode_);
}

void Mode::write(char c)
{
    std::visit([c](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.write(c);
        }
    }, mode_);
}

void Mode::clear()
{
    std::visit([](auto&& mode) {
        mode.clear();
    }, mode_);
}

void Mode::move_cursor(int row, int column)
{
    std::visit([row, column](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.move_cursor(row, column);
        }
    }, mode_);
}

void Mode::set_cursor(int row, int column)
{
    std::visit([row, column](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.set_cursor(row, column);
        }
    }, mode_);
}

void Mode::set_cursor_row(int row)
{
    std::visit([row](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.set_cursor_row(row);
        }
    }, mode_);
}

void Mode::set_cursor_column(int column)
{
    std::visit([column](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.set_cursor_column(column);
        }
    }, mode_);
}

void Mode::set_foreground_color(int foreground)
{
    std::visit([foreground](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.set_foreground_color(foreground);
        }
    }, mode_);
}

void Mode::set_background_color(int background)
{
    std::visit([background](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.set_background_color(background);
        }
    }, mode_);
}

void Mode::set_color(int foreground, int background)
{
    std::visit([foreground, background](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Text>::value)
        {
            mode.set_color(foreground, background);
        }
    }, mode_);
}

void Mode::set_pixel(int x, int y, uint16_t color)
{
    std::visit([x, y, color](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Graphic>::value)
        {
            mode.set_pixel(x, y, color);
        }
    }, mode_);
}

void Mode::draw_line(int x1, int y1, int x2, int y2)
{
    std::visit([x1, y1, x2, y2](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Graphic>::value)
        {
            mode.draw_line(x1, y1, x2, y2);
        }
    }, mode_);
}

void Mode::begin_primitives(PrimitiveType type)
{
    std::visit([type](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Graphic>::value)
        {
            mode.start_primitives(type);
        }
    }, mode_);

}

void Mode::end_primitives()
{
    std::visit([](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Graphic>::value)
        {
            mode.end_primitives();
        }
    }, mode_);
}

void Mode::write_vertex(float x, float y, float z)
{
    std::visit([x, y, z](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Graphic>::value)
        {
            mode.write_vertex(x, y, z);
        }
    }, mode_);
}

void Mode::set_perspective(float angle, float aspect, float z_far, float z_near)
{
    std::visit([angle, aspect, z_far, z_near](auto&& mode) {
        if constexpr (std::is_same<typename std::decay_t<decltype(mode)>::type, vga::modes::Graphic>::value)
        {
            mode.set_perspective(angle, aspect, z_far, z_near);
        }
    }, mode_);

}

void Mode::swap_buffer() 
{
    std::visit([](auto&& mode) {
        if constexpr (std::decay_t<decltype(mode)>::ConfigurationType::double_buffered)
        {
            mode.swap_buffers();
        }
    }, mode_);
}


} // namespace vga
