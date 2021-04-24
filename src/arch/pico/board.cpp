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

#include "board.hpp"

#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>

#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <pico/sync.h>

#include <hardware/clocks.h>

extern const struct scanvideo_pio_program video_24mhz_composable;

static struct mutex frame_logic_mutex;
static struct semaphore video_setup_complete;

namespace msgpu 
{

void initialize_board()
{
    set_sys_clock_khz(250000, true);
    stdio_init_all();
}

void initialize_signal_generator()
{
    mutex_init(&frame_logic_mutex);
    sem_init(&video_setup_complete);
    multicore_launch_core1(core1_func);
}

} // namespace msgpu 
