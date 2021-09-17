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

namespace msgpu::generator
{

Vga::Vga(modes::Modes mode)
    : vram_(nullptr)
{
    mutex_init(&vga_mutex_);
    if (mode == modes::Modes::Graphic_320x240_12bit)
    {
        set_resolution(320, 240);
    }
}

void Vga::change_mode(modes::Modes mode)
{
}

void Vga::setup(memory::VideoRam *vram)
{
    vram_ = vram;
}

std::size_t Vga::display_line(std::size_t line, std::span<uint32_t> to_display)

{
    static uint16_t buffer[320] = {};
    if (vram_)
    {
        vram_->read_line(line, buffer);
    }
    std::span<uint16_t> scanline_buffer(buffer);
    std::transform(scanline_buffer.begin(), scanline_buffer.end(), to_display.begin(),
                   [](uint16_t color) {
                       return color; // TODO: transform?
                   });

    return 0;
}

void Vga::block()
{
    vram_->block();
}

void Vga::unblock()
{
    vram_->unblock();
}

Vga &get_vga()
{
    static Vga vga(modes::Modes::Graphic_320x240_12bit);
    return vga;
}

} // namespace msgpu::generator
