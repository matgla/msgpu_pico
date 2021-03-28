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
#include <hardware/structs/vreg_and_chip_reset.h>

#include "processor/command_processor.hpp"
#include "processor/human_interface.hpp"


extern const struct scanvideo_pio_program video_24mhz_composable;

static struct mutex frame_logic_mutex;

static void frame_update_logic();
static void render_scanline(struct scanvideo_scanline_buffer* dest, int core);

//static vga::Vga vga_generator(&vga_mode_640x480_60);

vga::Mode* global_mode;

void render_loop()
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

void frame_update_logic() 
{
    if (global_mode)
    {
        global_mode->render();
    }
}

uint16_t current_line_[640];

void render_scanline(struct scanvideo_scanline_buffer* dest, int core)
{

    int l = scanvideo_scanline_number(dest->scanline_id);
    if (global_mode)
    {
        std::size_t size = global_mode->fill_scanline(std::span<uint32_t>(dest->data, dest->data_max), l);
        std::memset(current_line_, 0xff, sizeof(current_line_));
        std::span<uint32_t> line(dest->data, dest->data_max); 
    static uint32_t postamble[] = {
        0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
    };

    //line[0] = 4;
    //line[1] = host_safe_hw_ptr(line.data() + 8);
    //line[2] = (640 - 4) / 2;
    //line[3] = host_safe_hw_ptr(current_line_ + 4);
    //line[4] = count_of(postamble);
    //line[5] = host_safe_hw_ptr(postamble);
   // line[6] = 0;
    //line[7] = 0;

    //line[8] = (current_line_[0] << 16u) | COMPOSABLE_RAW_RUN;
    //line[9] = (current_line_[1] << 16u) | 0;
    //line[10] = (COMPOSABLE_RAW_RUN << 16u) | current_line_[2];
    //line[11] = (640 - 4 << 16u) | current_line_[3];
    
    dest->data_used = size;
    dest->status = SCANLINE_OK;
    }
}

void core1_func() 
{
    sem_acquire_blocking(&video_setup_complete);
    render_loop();
}


int vga_main()
{
    mutex_init(&frame_logic_mutex);
    sem_init(&video_setup_complete, 0, 1);

    multicore_launch_core1(core1_func);

    return 0;
}

int main() 
{
    set_sys_clock_khz(200000, true);
    stdio_init_all();
    vga::Vga vga(&vga_mode_640x480_60); 
    vga::Mode mode(vga);
    mode.switch_to(vga::Modes::Text_80x25);
    global_mode = &mode; 
    processor::CommandProcessor processor(mode);
    processor.change();
   
    vga_main();
    
    vga.setup();
    
    sem_release(&video_setup_complete);

    while (true)
    {
        uint8_t byte; 
        read(STDIN_FILENO, &byte, sizeof(uint8_t));
        uint8_t t = 'c';
        write(STDOUT_FILENO, &t, sizeof(uint8_t)); 
        processor.process(byte);
    }
    while (true)
    {
    }
} 

