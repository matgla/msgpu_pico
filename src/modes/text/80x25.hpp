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

#include <cstddef>

#include <pico/scanvideo.h>

namespace vga
{
namespace modes
{
namespace text
{

template <typename Font>
struct Text_80x25_16color
{
    constexpr static std::size_t width = 80;
    constexpr static std::size_t height = 25;
    constexpr static std::size_t bits_per_pixel = 4; 
    const static inline scanvideo_mode_t* mode = &vga_mode_640x480_60;
};

} // namespace text
} // namespace modes
} // namespace vga



