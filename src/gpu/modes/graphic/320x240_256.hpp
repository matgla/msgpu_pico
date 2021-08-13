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

#include <array>
#include <cstddef>
#include <cstdint>

#include "generator/modes.hpp"

namespace msgpu::modes::graphic 
{

class Graphic_320x240_256
{
public:
    using ColorType = uint8_t; 
    
    constexpr static std::size_t resolution_width = 320;
    constexpr static std::size_t resolution_height = 240;

    constexpr static std::size_t width = resolution_width;
    constexpr static std::size_t height = resolution_height;

    constexpr static bool uses_color_palette = false;
    constexpr static std::size_t bits_per_pixel = 8;

    constexpr static Modes mode = Modes::Graphic_320x240_12bit;
    constexpr static bool double_buffered = true;
    
    enum Color : ColorType {
        black = 0x00, 
        blue = 0x03,
        green = 0x1c,
        cyan = 0x12, 
        red = 0xc0, 
        mageta = 0xee,
        orange = 0xdd, 
        grey = 0x12, 
        dark_grey = 0x77,
        bright_blue = 0x66,
        bright_green = 0xf6, 
        bright_cyan = 0xf6, 
        bright_red = 0x2f, 
        bright_magneta = 0xff, 
        yellow = 0xff, 
        white = 0xff
    };
};

} // namespace msgpu::modes::graphic
