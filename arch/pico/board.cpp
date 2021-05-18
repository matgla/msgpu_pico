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
static eul::container::static_deque<uint8_t, 255> buffer;

void on_uart_rx() 
{
    while (uart_is_readable(uart0)) 
    {
        //buffer.push_back(uart_getc(uart0));
        uart_putc(uart0, uart_getc(uart0));
    }
}

const char word0[] = "Transferring data ";
const char word1[] = "one ";
const char word2[] = "at ";
const char word3[] = "a time!\n";

const struct {uint32_t len; const char* data;} control_blocks[] = {
    {count_of(word0) - 1, word0},
    {count_of(word1) - 1, word1},
    {count_of(word2) - 1, word2},
    {count_of(word3) - 1, word3},
    {0, NULL}
};

void initialize_uart()
{
    stdio_init_all();
    uart_init(uart0, 115200);
    uart_set_hw_flow(uart0, true, true);
    int UART_IRQ = UART0_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_fifo_enabled(uart0, false);
    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(uart0, true, false);
    gpio_set_function(16, GPIO_FUNC_UART);
    gpio_set_function(17, GPIO_FUNC_UART);
    gpio_set_function(18, GPIO_FUNC_UART);
    gpio_set_function(19, GPIO_FUNC_UART);
}

static int dma_channel;

UsartHandler usart_dma_handler;

void set_usart_dma_buffer(uint8_t* buffer, bool trigger)
{
    dma_channel_set_write_addr(dma_channel, buffer, trigger);  
}

void set_usart_dma_transfer_count(std::size_t size, bool trigger)
{
    dma_channel_set_trans_count(dma_channel, size, trigger);
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
 //   if (got_header)
 //   {
 //       printf("Got header\n");
 //       got_header = false;
 //       printf("Data to get: %d\n", dst[0]);
 //       dma_channel_set_write_addr(dma_channel, dst, false);
 //       dma_channel_set_trans_count(dma_channel, dst[0], true);
 //   }
 //   else 
 //   {
 //       printf("Got payload: %s\n", dst);
 //       dma_channel_set_write_addr(dma_channel, dst, false);
 //       dma_channel_set_trans_count(dma_channel, 4, true);
 //       got_header = true;
 //   }

    
}

void dma_test()
{
    dma_channel = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, DREQ_UART0_RX);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);

    dma_channel_set_irq1_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler);  
    irq_set_enabled(DMA_IRQ_1, true);

    dma_channel_configure( 
        dma_channel, 
        &c, 
        nullptr, 
        &uart_get_hw(uart0)->dr, 
        4,
        false 
    );

    printf("Waiting for data\n");
}

void initialize_board()
{
    set_sys_clock_khz(250000, true);
    initialize_uart();

    // dma_test();


//    int ctrl_chan = dma_claim_unused_channel(true);
//    int data_chan = dma_claim_unused_channel(true);

//    dma_channel_config c = dma_channel_get_default_config(ctrl_chan);
//    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
//    channel_config_set_read_increment(&c, true);
//    channel_config_set_write_increment(&c, true);
//    channel_config_set_ring(&c, true, 3);

//    dma_channel_configure(
//        ctrl_chan,
//        &c, 
//        &dma_hw->ch[data_chan].al3_transfer_count,
//        &control_blocks[0],
//        2,
//        false 
//    );

//    c = dma_channel_get_default_config(data_chan);
//    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
//    channel_config_set_dreq(&c, DREQ_UART0_TX + 2 * uart_get_index(uart0));
//    channel_config_set_chain_to(&c, ctrl_chan);
//    channel_config_set_irq_quiet(&c, true);
//    dma_channel_configure( 
//        data_chan,
//        &c,
//        &uart_get_hw(uart0)->dr, 
//        NULL,
//        0,
//        false
//    );
//    dma_start_channel_mask(1u << ctrl_chan);
//    while (!(dma_hw->intr & 1u << data_chan))
//        tight_loop_contents();
//    dma_hw->ints0 = 1u << data_chan;
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

        mutex_enter_blocking(&frame_logic_mutex);

        uint32_t frame_num = scanvideo_frame_number(scanline_buffer->scanline_id);

        static uint32_t line = 0;

        if (line == res_height)
        {
            frame_update();
            line = 0; 
        }
        ++line;

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
    res_width = width;
    res_height = height;
}

uint8_t read_byte()
{
    return uart_getc(uart0);
    //while (buffer.empty())
    //{
    //}
    //uint8_t b = buffer.front();
    //b.pop_front();
    //return b;
}

void write_bytes(std::span<uint8_t> bytes)
{
    for (const auto b : bytes)
    {
        uart_putc_raw(uart0, b);
    }
//    write(STDOUT_FILENO, bytes.data(), bytes.size());
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
