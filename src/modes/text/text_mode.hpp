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

#include <span>

#include <cstring>

#include <msgui/Position.hpp>

#include "generator/vga.hpp"

#include "modes/types.hpp"
#include "modes/buffer.hpp"

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
        vga.change_mode(Configuration::mode);
        vga.setup();
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

    std::size_t fill_scanline(std::span<uint32_t> line, std::size_t line_number)
    {
        std::memset(current_line_, 0xf, sizeof(current_line_));
        
        static uint32_t postamble[] = {
            0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
        };

        line[0] = 4;
        line[1] = host_safe_hw_ptr(line.data() + 8);
        line[2] = (Configuration::mode->width - 4) / 2;
        line[3] = host_safe_hw_ptr(current_line_ + 4);
        line[4] = count_of(postamble);
        line[5] = host_safe_hw_ptr(postamble);
        line[6] = 0;
        line[7] = 0;

        line[8] = (current_line_[0] << 16u) | COMPOSABLE_RAW_RUN;
        line[9] = (current_line_[1] << 16u) | 0;
        line[10] = (COMPOSABLE_RAW_RUN << 16u) | current_line_[2];
        line[11] = (Configuration::mode->width - 2 << 16u) | current_line_[3];
        
        return 8;
    }

    // RENDERING
    
    void render()
    {
    }

   
private:
    char text_buffer_[Configuration::height][Configuration::width];

    uint16_t current_line_[Configuration::width];
};

} // namespace text
} // namespace modes
} // namespace vga



