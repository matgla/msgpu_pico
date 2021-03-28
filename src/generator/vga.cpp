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

#include <pico/scanvideo.h>
#include <hardware/dma.h>

#include "memory/video_ram.hpp"

namespace vga
{

Vga::Vga(const scanvideo_mode_t* mode)
    : mode_(mode)
{
    scanvideo_setup(mode_);
    scanvideo_timing_enable(true);
}

void Vga::setup()
{
    return; 
    dma_channel_unclaim(0);
    //dma_set_irq0_channel_mask_enabled(1, false);
    //dma_channel_unclaim(0);
    pio_clear_instruction_memory(pio0);
    scanvideo_setup(mode_);
    scanvideo_timing_enable(true);
}

void Vga::change_mode(const scanvideo_mode_t* mode)
{
    mode_ = mode;
}

bool Vga::is_vsync() const
{
    return false;
}

bool Vga::render() const
{
    return false;
}

void Vga::render(bool enable)
{
}

std::size_t Vga::get_width() const
{
    return mode_->width;
}

} // namespace vga
