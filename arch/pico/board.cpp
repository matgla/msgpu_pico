// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
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
#include <hardware/regs/intctrl.h>
#include <hardware/structs/uart.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <eul/container/static_deque.hpp>

#include "hal_dma.hpp"

#include "generator/vga.hpp"

extern const struct scanvideo_pio_program video_24mhz_composable;

static struct mutex frame_logic_mutex;
static struct semaphore video_setup_complete;

namespace msgpu 
{

#include <cstdio>

static uint32_t res_width = 320;
static uint32_t res_height = 240;
std::array<uint8_t, 32> buffer;

void on_uart_rx() 
{
    while (uart_is_readable(uart0)) 
    {
        //buffer.push_back(uart_getc(uart0));
        uart_putc(uart0, uart_getc(uart0));
    }
}


//void __time_critical_func(render_scanline)(scanvideo_scanline_buffer* dest, int core)
//{
//    int l = scanvideo_scanline_number(dest->scanline_id);

//    auto line = get_scanline(l);
//    std::size_t size = get_vga().display_line(std::span<uint32_t>(dest->data, dest->data_max), line);

//    dest->data_used = size;
//    dest->status = SCANLINE_OK;
//}

//void __time_critical_func(render_loop)()
//{
//    int core_num = get_core_num();
//    while (true)
//    {
//        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(true);

//        uint32_t frame_num = scanvideo_frame_number(scanline_buffer->scanline_id);

//        static uint32_t line = 0;

//        if (line == res_height)
//        {
//            frame_update();
//            line = 0; 
//        }
//        ++line;

//        mutex_enter_blocking(&frame_logic_mutex);
//        render_scanline(scanline_buffer, core_num);
//        mutex_exit(&frame_logic_mutex); 
//        scanvideo_end_scanline_generation(scanline_buffer);
//    }
//}

void core1_func()
{
    sem_acquire_blocking(&video_setup_complete);
    //render_loop();
}

int64_t timer_callback(alarm_id_t alarm_id, void* user_data)
{
    //render_loop();
    return 20;
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
    res_width = width;
    res_height = height;
}

uint8_t read_byte()
{
    return uart_getc(uart0);
}

void write_bytes(std::span<const uint8_t> bytes)
{
    for (const auto b : bytes)
    {
        uart_putc_raw(uart0, b);
    }
}


uint32_t get_millis()
{
    return to_ms_since_boot(get_absolute_time());
}

uint64_t get_us()
{
    return to_us_since_boot(get_absolute_time());
}

void sleep_ms(uint32_t t)
{
    ::sleep_ms(t);
}

void sleep_us(uint32_t t)
{
    ::sleep_us(t);
}

} // namespace msgpu 
