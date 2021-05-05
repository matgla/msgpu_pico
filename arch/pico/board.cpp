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

#include <unistd.h>
#include <fcntl.h>

#include <cstdio>

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

void __time_critical_func(render_scanline)(scanvideo_scanline_buffer* dest, int core)
{
    int l = scanvideo_scanline_number(dest->scanline_id);

    std::size_t size = fill_scanline(std::span<uint32_t>(dest->data, dest->data_max), l);

    dest->data_used = size;
    dest->status = SCANLINE_OK;
}

void __time_critical_func(render_loop)()
{
    static uint32_t last_frame_num = 0;
    int core_num = get_core_num();

    while (true) 
    {
        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(true);

        mutex_enter_blocking(&frame_logic_mutex);

        uint32_t frame_num = scanvideo_frame_number(scanline_buffer->scanline_id);

        if (frame_num != last_frame_num)
        {
            last_frame_num = frame_num;
            msgpu::frame_update();
        }

        render_scanline(scanline_buffer, core_num);
        mutex_exit(&frame_logic_mutex);
        
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

void core1_func()
{
    sem_acquire_blocking(&video_setup_complete);

    render_loop();
}

void block_display()
{
    mutex_enter_blocking(&frame_logic_mutex);
}

void unblock_display()
{
    mutex_exit(&frame_logic_mutex);
}

void initialize_signal_generator()
{
    mutex_init(&frame_logic_mutex);
    sem_init(&video_setup_complete, 0, 1);
    multicore_launch_core1(core1_func);
    start_vga();

    sem_release(&video_setup_complete);
}

void set_resolution(uint16_t width, uint16_t height)
{

}

uint8_t read_byte()
{
    uint8_t byte;
    read(STDIN_FILENO, &byte, sizeof(byte));
    return byte;
}

void write_bytes(std::span<uint8_t> bytes)
{
    write(STDOUT_FILENO, bytes.data(), bytes.size());
}

} // namespace msgpu 
