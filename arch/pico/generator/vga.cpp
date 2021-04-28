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

#include "generator/vga.hpp"

#include <cstring>
#include <cstdio> 

#include <pico/scanvideo.h>
#include <hardware/dma.h>
#include <hardware/sync.h>

#include "memory/video_ram.hpp"

namespace vga
{

const scanvideo_mode_t* convert_mode(const modes::Modes mode)
{
    switch (mode) 
    {
        case Modes::Text_40x30_12bit:
        case Modes::Text_40x30_16:
        {
            return &vga_mode_320x240_60;
        } break;
        case Modes::Text_80x30_16:
        {
            return &vga_mode_640x480_60;
        }
    }
    return &vga_mode_640x480_60;
}


Vga::Vga(modes::Modes mode)
    : mode_(convert_mode(mode))
{
    scanvideo_setup(mode_);
    scanvideo_timing_enable(true);
}

void Vga::setup()
{
    uint32_t mask = save_and_disable_interrupts();
    scanvideo_change_mode(mode_);

    scanvideo_timing_enable(true);
    restore_interrupts(mask);
}

void Vga::change_mode(modes::Modes mode)
{
    mode_ = convert_mode(mode);
    setup();
}

Vga& get_vga() 
{
    static Vga vga(modes::Modes::Text_80x30_16);
    return vga;
}

} // namespace vga
