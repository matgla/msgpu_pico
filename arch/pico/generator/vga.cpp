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
#include <pico/scanvideo/composable_scanline.h>

#include <hardware/dma.h>
#include <hardware/sync.h>

#include "memory/video_ram.hpp"

#include "board.hpp"

namespace vga
{

namespace 
{
static const scanvideo_mode_t* mode_;
}

const scanvideo_mode_t* convert_mode(const modes::Modes mode)
{
    switch (mode) 
    {
        case modes::Modes::Text_40x30_12bit:
        case modes::Modes::Text_40x30_16:
        {
            return &vga_mode_320x240_60;
        } break;
        case modes::Modes::Text_80x30_16:
        {
            return &vga_mode_640x480_60;
        }
    }
    return &vga_mode_640x480_60;
}


Vga::Vga(modes::Modes mode)
{
    mode_ = convert_mode(mode);
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

std::size_t Vga::fill_scanline_buffer(std::span<uint32_t> line, std::span<const uint16_t> scanline_buffer)
{
    const uint16_t* current_line = scanline_buffer.data();

    static uint32_t postamble[] = {
        0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
    };

    line[0] = 4;
    line[1] = host_safe_hw_ptr(line.data() + 8);
    line[2] = (mode_->width - 4) / 2;
    line[3] = host_safe_hw_ptr(current_line + 4);
    line[4] = count_of(postamble);
    line[5] = host_safe_hw_ptr(postamble);
    line[6] = 0;
    line[7] = 0;

    line[8] = (current_line[0] << 16u) | COMPOSABLE_RAW_RUN;
    line[9] = (current_line[1] << 16u) | 0;
    line[10] = (COMPOSABLE_RAW_RUN << 16u) | current_line[2]; 
    line[11] = ((mode_->width - 5) << 16u) | current_line[3];
    return 8;
}

Vga& get_vga() 
{
    static Vga vga(modes::Modes::Text_80x30_16);
    return vga;
}

} // namespace vga
