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

#include "hal_dma.hpp"

#include <cstdio>
#include <cstdint>

#include <pico/stdlib.h>

#include <hardware/dma.h>
#include <hardware/irq.h>

namespace hal
{
namespace 
{
static int dma_channel;

static UsartHandler usart_dma_handler;
constexpr int dma_prio = 0x10;


}

void dma_handler()
{
    dma_hw->ints0 = 1 << dma_channel;
    if (usart_dma_handler)
    {
        usart_dma_handler();
    }
}

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



void set_usart_handler(const UsartHandler& handler)
{
    usart_dma_handler = handler;
}



} // namespace hal
