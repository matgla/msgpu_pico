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

#include <cstddef>

#include "modes/mode_types.hpp"

namespace vga
{
namespace modes
{
namespace text
{

template <typename Font>
struct Text_40x30_12bit
{
    using font = Font;
    constexpr static std::size_t width = 40;
    constexpr static std::size_t height = 30;
    constexpr static std::size_t bits_per_pixel = 12; 
    using ColorType = uint16_t;
    constexpr static Modes mode = Modes::Text_40x30_12bit;
    constexpr static std::size_t resolution_width = 320;
    constexpr static std::size_t resolution_height = 240;
    constexpr static bool uses_color_palette = false;

    enum Color : ColorType {
        black = 0x000, 
        blue = 0x00f,
        green = 0x0f0,
        cyan = 0x0ff, 
        red = 0xf00, 
        mageta = 0xf0f,
        orange = 0xff6, 
        grey = 0x777, 
        dark_grey = 0x888,
        bright_blue = 0x00f,
        bright_green = 0x0f0, 
        bright_cyan = 0x0ff, 
        bright_red = 0xf00, 
        bright_magneta = 0xf0f, 
        yellow = 0xff0, 
        white = 0xfff
    };
};

} // namespace text
} // namespace modes
} // namespace vga



