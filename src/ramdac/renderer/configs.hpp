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

namespace msgpu::renderer 
{

enum class ColorSpace
{
    Palette_16,
    Palette_256,
    Rgb332,
    Rgb444
};

struct ModeConfig 
{
    VgaModes vga_mode;
    ColorSpace color_space;
};

constexpr ModeConfig configs[] = {
    /* 0 320x240@8bit */ { 
        .vga_mode = VgaModes::r320x240_60,
        .color_space = ColorSpace::Rgb444
    }
};

} // namespace msgpu::renderer
