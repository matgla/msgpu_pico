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

namespace vga::modes 
{

enum class Modes
{
    Text_80x30_16,
    Text_40x30_16,
    Text_40x30_12bit,
    Graphic_640x480_16,
    Graphic_320x240_16,
    Graphic_320x240_12bit
};

std::string_view to_string(Modes mode);

} // namespace vga::modes

