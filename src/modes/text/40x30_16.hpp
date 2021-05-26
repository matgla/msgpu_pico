// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
//
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

#include <array>
#include <cstdint>
#include <cstddef>

#include "modes.hpp"

namespace vga
{
namespace modes
{
namespace text
{

template <typename Font>
struct Text_40x30_16color
{
    using font = Font;
    constexpr static std::size_t width = 40;
    constexpr static std::size_t height = 30;
    constexpr static std::size_t bits_per_pixel = 4; 
    using ColorType = uint8_t;
    constexpr static Modes mode = Modes::Text_40x30_16;
    constexpr static std::size_t resolution_width = 320;
    constexpr static std::size_t resolution_height = 240;
    constexpr static bool uses_color_palette = true;
    constexpr static bool double_buffered = false; 

    enum Color : ColorType {
        black = 0x0, 
        blue = 0x1,
        green = 0x2,
        cyan = 0x3, 
        red = 0x4, 
        mageta = 0x5,
        orange = 0x6, 
        grey = 0x7, 
        dark_grey = 0x8,
        bright_blue = 0x9,
        bright_green = 0xa, 
        bright_cyan = 0xb, 
        bright_red = 0xc, 
        bright_magneta = 0xd, 
        yellow = 0xe, 
        white = 0xf
    };

    static inline std::array<uint16_t, 16> color_palette = {
        0x000, // black         0x00
        0xc00, // blue          0x01
        0x0c0, // green         0x02
        0xcc0, // cyan          0x03
        0x00c, // red           0x04
        0xc0c, // magneta       0x05
        0x08c, // orange        0x06
        0xbbb, // grey          0x07
        0x777, // dark grey     0x08
        0xf66, // bright blue   0x09
        0x6f6, // bright green  0x0a
        0xff6, // bright cyan   0x0b
        0x22f, // bright red    0x0c
        0xf6f, // bright magneta 0x0d
        0x0ff, // yellow        0x0e
        0xfff  // white         0x0f
    };
};

} // namespace text
} // namespace modes
} // namespace vga



