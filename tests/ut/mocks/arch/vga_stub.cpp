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

#include "generator/vga.hpp"

#include "board.hpp"

#include <algorithm>
#include <cstring>

#include <eul/utils/unused.hpp>

namespace msgpu::generator
{

Vga::Vga(modes::Modes mode)
    : vram_(nullptr)
{
    UNUSED1(mode);
}

void Vga::change_mode(modes::Modes mode)
{
    UNUSED1(mode);
}

void Vga::setup(memory::VideoRam *vram)
{
    vram_ = vram;
}

std::size_t Vga::display_line(std::size_t line, std::span<uint32_t> to_display)

{
    UNUSED2(line, to_display);
    return 0;
}

void Vga::block()
{
}

void Vga::unblock()
{
}

Vga &get_vga()
{
    static Vga vga(modes::Modes::Graphic_320x240_12bit);
    return vga;
}

} // namespace msgpu::generator
