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

#include "generator/vga.hpp"

#include "board.hpp"

#include <algorithm>
#include <cstring>

namespace msgpu 
{

Vga::Vga(modes::Modes mode) 
{
    if (mode == modes::Modes::Graphic_320x240_12bit)
    {
        set_resolution(320, 240);
    }
}

void Vga::change_mode(modes::Modes mode)
{
}

void Vga::setup()
{

}

std::size_t Vga::display_line(std::span<uint32_t> line, 
    std::span<const uint8_t> scanline_buffer)
{
    std::transform(scanline_buffer.begin(), scanline_buffer.end(), line.begin(), [](uint8_t color) {
        return color; // TODO: transform?
    });
    return 0;
}

Vga& get_vga()
{
    static Vga vga(modes::Modes::Graphic_320x240_12bit);
    return vga;
}

} // namespace msgpu 
