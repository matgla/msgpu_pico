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

namespace msgpu::modes 
{

enum class Modes
{
    Text_80x30_16 = 1,
    Text_40x30_16 = 2,
    Text_40x30_12bit = 3,
    Graphic_640x480_16 = 10,
    Graphic_320x240_16 = 11,
    Graphic_320x240_12bit = 12
};

constexpr const char* to_string(Modes m)
{
    switch (m)
    {
        case Modes::Text_80x30_16: return "Text 80x30 - 16 colors";
        case Modes::Text_40x30_16: return "Text 40x30 - 16 colors";
        case Modes::Text_40x30_12bit: return "Text 40x30 - RGB444";
        case Modes::Graphic_640x480_16: return "Graphic 640x480 - 16 colors";
        case Modes::Graphic_320x240_16: return "Graphic 320x240 - 16 colors";
        case Modes::Graphic_320x240_12bit: return "Graphic 320x240 - RGB444";
    }
    return "Unknown";
}

} // namespace msgpu::modes

