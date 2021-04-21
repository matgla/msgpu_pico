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

#include "modes/mode_base.hpp"

#include "generator/vga.hpp"

namespace vga::modes 
{

void vga_change_mode(const Modes mode)
{

    switch (mode) 
    {
        case Modes::Text_40x30_12bit:
        case Modes::Text_40x30_16:
        {
            get_vga().change_mode(&vga_mode_320x240_60);
        } break;
        case Modes::Text_80x30_16:
        {
            get_vga().change_mode(&vga_mode_640x480_60);
        }
    }

    get_vga().setup();
}

} // namespace vga::modes

