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

#define PICO_SCANVIDEO_COLOR_PIN_COUNT 12

#include <pico.h> 
#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>
#include <pico/multicore.h>
#include <pico/sync.h> 
#include <pico/stdlib.h>

#define DUAL_CORE_RENDER
#define VGA_MODE vga_mode_640x480_60

static_assert(PICO_SCANVIDEO_COLOR_PIN_COUNT == 12);
static_assert(PICO_SCANVIDEO_SYNC_PIN_BASE == 12);

extern const struct scanvideo_pio_program video_24mhz_composable;

static struct mutex frame_logic_mutex;

static void frame_update_logic();
static void render_scanline(struct scanvideo_scanline_buffer* dest, int core);

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

void frame_update_logic() 
{
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

void render_scanline(struct scanvideo_scanline_buffer* dest, int core)
{
    color = 0xfff;
    
    draw_line(dest, 0x0001);
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
    render_loop();
    return 0;
}

int main() 
{
//    #if PICO_SCANVIDEO_48MHz
//        set_sys_clock_48mhz();
//    #endif 

    stdio_init_all();
    return vga_main();

} 

