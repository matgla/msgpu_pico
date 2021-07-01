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

#include <cstdint>
#include <cstdio> 

#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/dma.h>
#include <hardware/structs/bus_ctrl.h>

#include "qspi.pio.h"

#include "qspi.hpp"

namespace 
{

constexpr PIO __time_critical_func(get_pio)(const uint32_t number)
{
    switch (number) 
    {
        case 0: return pio0; 
        case 1: return pio1;
    }
    return {};
}

volatile io_rw_8* get_tx_fifo(PIO pio, uint32_t sm)
{
    return reinterpret_cast<io_rw_8*>(&pio->txf[sm]);
}

volatile io_ro_8* get_rx_fifo(PIO pio, uint32_t sm)
{
    return reinterpret_cast<io_ro_8*>(&pio->rxf[sm]);
}

static uint32_t qspi_data_program = 0;
constexpr int default_timeout = 10000;
static int dma_channel_1 = 0;
static int dma_channel_2 = 0;
}

Qspi::Qspi(const QspiConfig config, float clkdiv)
    : config_(config)
    , clkdiv_(clkdiv)
{
}

void Qspi::init() 
{
    qspi_data_program = pio_add_program(get_pio(config_.pio), &qspi_program);

    pio_qspi_init_data(get_pio(config_.pio),
        config_.sm,
        qspi_data_program,
        clkdiv_,
        config_.io_base,
        config_.sck,
        config_.cs
    );

    dma_channel_1 = dma_claim_unused_channel(true);
    dma_channel_2 = dma_claim_unused_channel(true);
    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
}

void __time_critical_func(Qspi::setup_dma_write)(ConstDataType src, int channel, int chain_to) 
{
    dma_channel_config c = dma_channel_get_default_config(channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(get_pio(config_.sm), config_.sm, true));

    if (chain_to >= 0)
    {
        channel_config_set_chain_to(&c, chain_to);
    }

    dma_channel_configure( 
        channel, 
        &c, 
        &get_pio(config_.pio)->txf[config_.sm], 
        src.data(),
        src.size(),
        false 
    ); 

}

void __time_critical_func(Qspi::setup_dma_read)(DataType dest, int channel, int chain_to)
{
    dma_channel_config c = dma_channel_get_default_config(channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(get_pio(config_.sm), config_.sm, false));

    if (chain_to >= 0)
    {
        channel_config_set_chain_to(&c, chain_to);
    }

    dma_channel_configure( 
        channel, 
        &c, 
        dest.data(), 
        &get_pio(config_.pio)->rxf[config_.sm],
        dest.size(),
        false 
    ); 


}

void __time_critical_func(Qspi::wait_for_finish)() const 
{
    dma_channel_wait_for_finish_blocking(dma_channel_1);
    dma_channel_wait_for_finish_blocking(dma_channel_2);
}

void Qspi::setup_dma_command_write(ConstDataType cmd, ConstDataType data)
{
    setup_dma_write(cmd, dma_channel_1, dma_channel_2);
    setup_dma_write(data, dma_channel_2);
}

void Qspi::setup_dma_command_read(ConstDataType cmd, DataType data)
{
    setup_dma_write(cmd, dma_channel_1, dma_channel_2);
    setup_dma_read(data, dma_channel_2);
}


bool Qspi::spi_transmit(ConstDataType src, DataType dest)
{
    if (dest.size() < src.size())
    {
        return false;
    }

    auto pio = get_pio(config_.pio);

    const uint8_t* s = src.data();
    uint8_t* d = dest.data();
    auto* tx = get_tx_fifo(pio, config_.sm);
    auto* rx = get_rx_fifo(pio, config_.sm);

    std::size_t tx_remain = src.size();
    std::size_t rx_remain = src.size();

    wait_until_previous_finished();

    pio_sm_set_in_pins(pio, config_.sm, config_.io_base + 1);
    pio_sm_put(pio, config_.sm, src.size() * 8 - 1);
    pio_sm_exec(pio, config_.sm, pio_encode_jmp(qspi_offset_spi_rw));

    int timeout = default_timeout;
    while (tx_remain || rx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(pio, config_.sm))
        {
            *tx = *s++;
            --tx_remain;
        }

        if (rx_remain && !pio_sm_is_rx_fifo_empty(pio, config_.sm))
        {
            *d++ = *rx;
            --rx_remain;
        }
        //if (--timeout == 0) return false;
    }
    return true;
}

bool Qspi::spi_read(DataType dest)
{
    auto pio = get_pio(config_.sm);
    uint8_t* d = dest.data();
    auto* tx = get_tx_fifo(pio, config_.sm);
    auto* rx = get_rx_fifo(pio, config_.sm);

    std::size_t tx_remain = dest.size();
    std::size_t rx_remain = dest.size();

    wait_until_previous_finished();
    
    pio_sm_set_in_pins(pio, config_.sm, config_.io_base + 1);
    pio_sm_put(pio, config_.sm, dest.size() * 8 - 1);
    pio_sm_exec(pio, config_.sm, pio_encode_jmp(qspi_offset_spi_rw));

    int timeout = default_timeout;
    while (tx_remain || rx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(pio, config_.sm))
        {
            *tx = 0;
            --tx_remain;
        }

        if (rx_remain && !pio_sm_is_rx_fifo_empty(pio, config_.sm))
        {
            *d++ = *rx;
            --rx_remain;
        }
        //if (--timeout == 0) return false;
    }
    return true;
}

bool Qspi::spi_write(ConstDataType src)
{
    auto pio = get_pio(config_.sm);
    const uint8_t* s = src.data();
    auto* tx = get_tx_fifo(pio, config_.sm);
    auto* rx = get_rx_fifo(pio, config_.sm);

    std::size_t tx_remain = src.size();
    std::size_t rx_remain = src.size();

    wait_until_previous_finished();

    pio_sm_put(pio, config_.sm, src.size() * 8 - 1);
    pio_sm_exec(pio, config_.sm, pio_encode_jmp(qspi_offset_spi_rw));

    int timeout = default_timeout;
    while (tx_remain || rx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(pio, config_.sm))
        {
            *tx = *s++;
            --tx_remain;
        }

        if (rx_remain && !pio_sm_is_rx_fifo_empty(pio, config_.sm))
        {
            static_cast<void>(*rx);
            --rx_remain;
        }
    }
    return true;
}

bool Qspi::qspi_write(ConstDataType src)
{
    auto pio = get_pio(config_.sm);
    const uint8_t* s = src.data();
    auto* tx = get_tx_fifo(pio, config_.sm);

    std::size_t tx_remain = src.size();

    wait_until_previous_finished();

    pio_sm_put(pio, config_.sm, src.size() * 2 - 1);
    pio_sm_exec(pio, config_.sm, pio_encode_jmp(qspi_offset_qspi_w));

    int timeout = default_timeout;
    while (tx_remain)
    {
        if (!pio_sm_is_tx_fifo_full(pio, config_.sm))
        {
            *tx = *s++;
            --tx_remain;
        }
    }
    return true;
}

bool __time_critical_func(Qspi::qspi_command_read)(DataType command, DataType data)
{
    uint8_t& wait_cycles = command.back();
    if (wait_cycles % 2 != 0) return false; 
    wait_cycles -= 1;

    auto pio = get_pio(config_.sm);

    wait_until_previous_finished();
   
    pio_sm_set_clkdiv(pio, config_.sm, 1.0f);
    setup_dma_write(command, dma_channel_1, dma_channel_2);
    setup_dma_read(data, dma_channel_2);

    pio_sm_set_in_pins(pio, config_.sm, config_.io_base);
    
    pio_sm_put(pio, config_.sm, data.size() * 2 - 1);

    dma_channel_start(dma_channel_1);
    pio_sm_exec(pio, config_.sm, pio_encode_jmp(qspi_offset_qspi_command_r));

    return true;
}

bool __time_critical_func(Qspi::qspi_command_write)(ConstDataType command, ConstDataType data)
{
    auto pio = get_pio(config_.sm);

    wait_until_previous_finished();
    
    pio_sm_set_clkdiv(pio, config_.sm, 1.0f);
    setup_dma_write(command, dma_channel_1, dma_channel_2);
    setup_dma_write(data, dma_channel_2);
    const int size = command.size() * 2 + data.size() * 2 - 1;
    pio_sm_put(pio, config_.sm, size);

    dma_channel_start(dma_channel_1);
    pio_sm_exec(pio, config_.sm, pio_encode_jmp(qspi_offset_qspi_w));

    return true;
}

bool __time_critical_func(Qspi::wait_until_previous_finished)()
{
    const int idle_wait_start = qspi_offset_idle_wait_loop;
    const int idle_wait_end = qspi_offset_idle_wait_end;
    auto pio = get_pio(config_.sm); 
    int timeout = 10000;
    while (pio->sm[config_.sm].addr < idle_wait_start
        || pio->sm[config_.sm].addr >= idle_wait_end) 
    {
    }

    return true;
}
