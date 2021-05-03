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

#include <cstddef>
#include <cstdint>

#include "modes/mode_types.hpp"

namespace vga::modes::graphic 
{

class Graphic_320x240_12bit 
{
public:
    using ColorType = uint16_t; 
    
    constexpr static std::size_t resolution_width = 320;
    constexpr static std::size_t resolution_height = 240;

    constexpr static std::size_t width = resolution_width;
    constexpr static std::size_t height = resolution_height;

    constexpr static bool uses_color_palette = false;
    constexpr static std::size_t bits_per_pixel = 12;

    constexpr static Modes mode = Modes::Graphic_320x240_12bit;

    enum Color : ColorType {
        black = 0x000, 
        blue = 0xc00,
        green = 0x0c0, 
        cyan = 0xcc0,
        red = 0x00c,
        magneta = 0xc0c,
        orange = 0x08c,
        grey = 0xbbb,
        dark_grey = 0x777,
        bright_blue = 0xf66,
        bright_green = 0x6f6,
        bright_cyan = 0xff6,
        bright_red = 0x22f,
        bright_magneta = 0xf6f,
        yellow = 0x0ff,
        white = 0xfff 
    };
};

} // namespace vga::modes::graphic

