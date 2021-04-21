// This file is part of MSGPU project.
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

#include <cstdio> 
#include <cstring> 

#include <unistd.h>

#include <pico.h> 
#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>
#include <pico/multicore.h>
#include <pico/sync.h> 
#include <pico/stdlib.h>
#include <hardware/clocks.h>

#include "processor/command_processor.hpp"
#include "processor/human_interface.hpp"

#include "disk/disk.hpp"

#include "config/config_manipulator.hpp"

extern const struct scanvideo_pio_program video_24mhz_composable;

static struct mutex frame_logic_mutex;

static void frame_update_logic();
static void render_scanline(struct scanvideo_scanline_buffer* dest, int core);

static vga::Mode* global_mode;

static int y = 0; 

void __time_critical_func(render_loop)()
{
    static uint32_t last_frame_num = 0;
    int core_num = get_core_num();

    while (true) 
    {
        struct scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(true);

        mutex_enter_blocking(&frame_logic_mutex);
        uint32_t frame_num = scanvideo_frame_number(scanline_buffer->scanline_id);

        if (frame_num != last_frame_num)
        {
            last_frame_num = frame_num; 
            frame_update_logic();
        
        }
        mutex_exit(&frame_logic_mutex);

        render_scanline(scanline_buffer, core_num);
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

struct semaphore video_setup_complete;

void __time_critical_func(frame_update_logic)() 
{
    if (global_mode)
    {
        global_mode->render();
    }
}

void __time_critical_func(render_scanline)(struct scanvideo_scanline_buffer* dest, int core) 
{
    int l = scanvideo_scanline_number(dest->scanline_id);
    if (global_mode)
    {
        std::size_t size = global_mode->fill_scanline(std::span<uint32_t>(dest->data, dest->data_max), l);
    
        dest->data_used = size;
        dest->status = SCANLINE_OK;
    }
}

void core1_func() 
{
    sem_acquire_blocking(&video_setup_complete);
    render_loop();
    printf("Core 1 finished\n");
}


int64_t timer_callback(alarm_id_t alarm_id, void *user_data) {
    struct scanvideo_scanline_buffer *buffer = scanvideo_begin_scanline_generation(false);
    while (buffer) {
        render_scanline(buffer, 0);
        scanvideo_end_scanline_generation(buffer);
        buffer = scanvideo_begin_scanline_generation(false);
    }
    return 100;
}

int vga_main()
{
    mutex_init(&frame_logic_mutex);
    sem_init(&video_setup_complete, 0, 1);

    multicore_launch_core1(core1_func);
    //add_alarm_in_us(100, timer_callback, NULL, true);
    return 0;
}

int main() 
{
    set_sys_clock_khz(250000, true);
    stdio_init_all();
 
    static vga::Mode mode(vga);
    mode.switch_to(vga::Modes::Text_80x30_16);
    global_mode = &mode; 
    static processor::CommandProcessor processor(mode);
    //processor.change();
    vga_main();
 
    vga.setup();
  
    sem_release(&video_setup_complete);
 
    uint8_t byte; 
    while (true)
    {
        read(STDIN_FILENO, &byte, sizeof(uint8_t));
        processor.process(byte);
    }
} 

