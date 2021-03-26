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

#define DUAL_CORE_RENDER
#define VGA_MODE vga_mode_640x480_60

static_assert(PICO_SCANVIDEO_COLOR_PIN_COUNT == 12);
static_assert(PICO_SCANVIDEO_SYNC_PIN_BASE == 12);

extern const struct scanvideo_pio_program video_24mhz_composable;

static struct mutex frame_logic_mutex;

static void frame_update_logic();
static void render_scanline(struct scanvideo_scanline_buffer* dest, int core);

vga::Mode* global_mode;

void render_loop()
{
    static uint32_t last_frame_num = 0;
    int core_num = get_core_num();

    printf("Rendering on core %d\n", core_num);
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

void clear_buffer()
{
}

int x = 0, y = 0;
void frame_update_logic() 
{
    clear_buffer();
}

#define MIN_COLOR_RUN 3
int32_t single_color_scanline(uint16_t* buf, size_t buf_length, int width, uint16_t color16)
{
    for (int i = 0; i < buf_length; ++i)
    {
        buf[i] = color16;
    }
    return buf_length;
}

uint16_t color = 0;

static uint16_t line_buffer[640];

void draw_line(struct scanvideo_scanline_buffer* buffer, uint16_t color)
{
    uint32_t line_num = scanvideo_scanline_number(buffer->scanline_id);
   
    uint32_t bar_width = VGA_MODE.width / 32;

    uint16_t *p = reinterpret_cast<uint16_t*>(buffer->data);
    
    for (int i = 0; i < 32; ++i)
    {
        *p++ = COMPOSABLE_COLOR_RUN;
        *p++ = color;
        *p++ = bar_width - 3;
        color = color << 1;
    }


    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;
    *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
    *p++ = 0;

    buffer->data_used = reinterpret_cast<uint32_t*>(p) - buffer->data;

    buffer->status = SCANLINE_OK;
}

static inline uint16_t* raw_scanline_prepare(scanvideo_scanline_buffer* dest, uint width)
{
    dest->data[0] = COMPOSABLE_RAW_RUN | (width + 1 - 3 << 16);
    dest->data[width / 2 + 2] = 0x0000u | (COMPOSABLE_EOL_ALIGN << 16);
    dest->data_used = width / 2 + 2;
    return (uint16_t*) &dest->data[1];
}

static inline void raw_scanline_finish(scanvideo_scanline_buffer* dest)
{
    uint32_t first = dest->data[0];
    uint32_t second = dest->data[1];

    dest->data[0] = (first & 0x0000ffffu) | ((second & 0x0000ffffu) << 16);
    dest->data[1] = (second & 0xffff0000u) | ((first & 0xffff0000u) >> 16);
    dest->status = SCANLINE_OK;
}


void render_scanline(struct scanvideo_scanline_buffer* dest, int core)
{
    uint16_t* color_buffer = raw_scanline_prepare(dest, VGA_MODE.width);

    int l = scanvideo_scanline_number(dest->scanline_id);
    if (global_mode)
    {
        global_mode->fill_scanline(std::span<uint16_t>(color_buffer, VGA_MODE.width), l);
    }

    raw_scanline_finish(dest);
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

#ifdef DUAL_CORE_RENDER 
    multicore_launch_core1(core1_func);
#endif

    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);

    sem_release(&video_setup_complete);
//    render_loop();
    return 0;
}

int main() 
{
    set_sys_clock_khz(200000, true);
     stdio_init_all();
    
    vga::Vga vga(&VGA_MODE);
    vga::Mode mode(vga);
    mode.switch_to(vga::Modes::Text_80x25);
    global_mode = &mode; 
    processor::CommandProcessor processor(mode);
    processor.change();
    
    vga_main();

    while (true)
    {
        uint8_t byte; 
        read(STDIN_FILENO, &byte, sizeof(uint8_t));
        uint8_t t = 'c';
        write(STDOUT_FILENO, &t, sizeof(uint8_t)); 
        processor.process(byte);
    }
} 

