// This file is part of msgput project.
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

<<<<<<< HEAD:src/arch/x86/board.cpp
#include "board.hpp"

namespace msgpu 
{

void initialize_board()
{

}

void initialize_signal_generator()
{

}

} // namespace msgpu 
=======
#include "modes/mode_types.hpp"

namespace vga::mode 
{

std::string_view to_string(Modes mode)
{
    switch (mode)
    {
        case Modes::Text_80x30_16: return "Text_80x30_16color";
        case Modes::Text_40x30_16: return "Text_40x30_16color";
        case Modes::Text_40x30_12bit: return "Text_40x30_12bit";
        case Modes::Graphic_640x480_16: return "Graphic_640x480_16color";
        case Modes::Graphic_320x240_16: return "Graphic_320x240_16color";
        case Modes::Graphic_320x240_12bit: return "Graphic_320x240_12bit";

    }
    return "Unknown";
}

} // namespace vga::mode
>>>>>>> 512f3036c96baa6b52ef91b776b90ab589666397:src/modes/mode_types.cpp

