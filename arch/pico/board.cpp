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

extern const struct scanvideo_pio_program video_24mhz_composable;

static struct mutex frame_logic_mutex;
static struct semaphore video_setup_complete;

namespace msgpu 
{

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

static int dma_channel;
constexpr int dma_size = 16;

const int dma_prio = 0x10;

void initialize_uart()
{
    uart_init(uart0, 230400);
    uart_set_hw_flow(uart0, false, false);
    uart_set_fifo_enabled(uart0, true);
    gpio_set_function(16, GPIO_FUNC_UART);
    gpio_set_function(17, GPIO_FUNC_UART);
 
  //  int UART_IRQ = UART0_IRQ;
  //  irq_set_enabled(UART_IRQ, true);
  //  irq_set_priority(UART_IRQ, uart_prio);
  //  irq_set_exclusive_handler(UART_IRQ, on_frame_finished);
   // uart_get_hw(uart0)->imsc = (1 << UART_UARTIMSC_RTIM_LSB);

    //hw_write_masked(&uart_get_hw(uart0)->ifls, 0 << UART_UARTIFLS_RXIFLSEL_LSB, UART_UARTIFLS_RXIFLSEL_BITS);
    //uart_set_irq_enables(uart0, true, true); 
}


UsartHandler usart_dma_handler;

void set_usart_dma_buffer(void* buffer, bool trigger)
{
    dma_channel_set_write_addr(dma_channel, buffer, trigger);  
}

void set_usart_dma_transfer_count(std::size_t size, bool trigger)
{
    printf("Setting dma count %d\n", size);
    dma_channel_set_trans_count(dma_channel, size, trigger);
    printf("Transfer count %d\n", dma_hw->ch[dma_channel].transfer_count);
}

void set_usart_handler(const UsartHandler& handler)
{
    usart_dma_handler = handler;
}

void dma_handler()
{
    dma_hw->ints0 = 1 << dma_channel;
    if (usart_dma_handler)
    {
        usart_dma_handler();
    }
}

void reset_dma_crc() 
{
    dma_hw->sniff_data = 0x0u;
}

void set_dma_mode(uint32_t mode)
{
    dma_sniffer_enable(dma_channel, mode, true); 
}

uint32_t get_dma_crc() 
{
    return dma_hw->sniff_data;
}

void enable_dma()
{
    dma_channel = dma_claim_unused_channel(true);
    
    dma_channel_config c = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, DREQ_UART0_RX + 2 * uart_get_index(uart0));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);

    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler);  
    irq_set_enabled(DMA_IRQ_1, true);
    irq_set_priority(DMA_IRQ_1, dma_prio);
    dma_channel_set_irq1_enabled(dma_channel, true);
   

    dma_channel_configure( 
        dma_channel, 
        &c, 
        nullptr, 
        &uart_get_hw(uart0)->dr, 
        0,
        false 
    );

    dma_sniffer_enable(dma_channel, 0x2, true);
}

void initialize_board()
{
    set_sys_clock_khz(250000, true);
    stdio_init_all();
    enable_dma();
    initialize_uart();


    printf("Board initialized\n");
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
    int core_num = get_core_num();
    while (true)
    {
        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(true);

        uint32_t frame_num = scanvideo_frame_number(scanline_buffer->scanline_id);

        static uint32_t line = 0;

        if (line == res_height)
        {
            frame_update();
            line = 0; 
        }
        ++line;

        mutex_enter_blocking(&frame_logic_mutex);
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

int64_t timer_callback(alarm_id_t alarm_id, void* user_data)
{
    render_loop();
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

void sleep_ms(uint32_t t)
{
    ::sleep_ms(t);
}


} // namespace msgpu 
