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

#include "qspi.pio.h"

#include "qspi.hpp"

float clkdiv = 31.25f;

namespace 
{
struct PinConfig 
{
    const uint32_t sck;
    const uint32_t io_base;
    const uint32_t cs;
    const uint32_t sm; 
    const PIO pio;
};

constexpr PinConfig get_config(const Qspi::Device device)
{
    switch (device)
    {
        case Qspi::Device::framebuffer: 
        {
            return {
                .sck = 26,
                .io_base = 18,
                .cs = 22,
                .sm = 0
            };
        }
    }
    return {};
}

constexpr PIO get_pio(const Qspi::Device device)
{
    switch (device) 
    {
        case Qspi::Device::framebuffer:
        {
            return pio1;
        }
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
}

Qspi::Qspi(const Device device, float clkdiv)
    : device_(device)
    , pin_sck_(get_config(device).sck)
    , pin_base_(get_config(device).io_base)
    , pin_cs_(get_config(device).cs)
    , sm_(get_config(device).sm)
    , clkdiv_(clkdiv)
{
}

void Qspi::init() 
{
    qspi_data_program = pio_add_program(get_pio(device_), &qspi_program);

    pio_qspi_init_data(get_pio(device_),
        sm_,
        qspi_data_program,
        clkdiv_,
        pin_base_,
        pin_sck_,
        pin_cs_
    );
}

bool Qspi::spi_transmit(ConstDataType src, DataType dest)
{
    if (dest.size() < src.size())
    {
        return false;
    }

    auto pio = get_pio(device_);

    const uint8_t* s = src.data();
    uint8_t* d = dest.data();
    auto* tx = get_tx_fifo(pio, sm_);
    auto* rx = get_rx_fifo(pio, sm_);

    std::size_t tx_remain = src.size();
    std::size_t rx_remain = src.size();

    wait_until_previous_finished();

    pio_sm_set_in_pins(pio, sm_, pin_base_ + 1);
    pio_sm_put(pio, sm_, src.size() * 8 - 1);
    pio_sm_exec(pio, sm_, pio_encode_jmp(qspi_offset_spi_rw));

    int timeout = default_timeout;
    while (tx_remain || rx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = *s++;
            --tx_remain;
        }

        if (rx_remain && !pio_sm_is_rx_fifo_empty(pio, sm_))
        {
            *d++ = *rx;
            --rx_remain;
        }
        if (--timeout == 0) return false;
    }
    return true;
}

bool Qspi::spi_read(DataType dest)
{
    auto pio = get_pio(device_);
    uint8_t* d = dest.data();
    auto* tx = get_tx_fifo(pio, sm_);
    auto* rx = get_rx_fifo(pio, sm_);

    std::size_t tx_remain = dest.size();
    std::size_t rx_remain = dest.size();

    wait_until_previous_finished();
    
    pio_sm_set_in_pins(pio, sm_, pin_base_ + 1);
    pio_sm_put(pio, sm_, dest.size() * 8 - 1);
    pio_sm_exec(pio, sm_, pio_encode_jmp(qspi_offset_spi_rw));

    int timeout = default_timeout;
    while (tx_remain || rx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = 0;
            --tx_remain;
        }

        if (rx_remain && !pio_sm_is_rx_fifo_empty(pio, sm_))
        {
            *d++ = *rx;
            --rx_remain;
        }
        if (--timeout == 0) return false;
    }
    return true;
}

bool Qspi::spi_write(ConstDataType src)
{
    auto pio = get_pio(device_);
    const uint8_t* s = src.data();
    auto* tx = get_tx_fifo(pio, sm_);
    auto* rx = get_rx_fifo(pio, sm_);

    std::size_t tx_remain = src.size();
    std::size_t rx_remain = src.size();

    wait_until_previous_finished();

    pio_sm_put(pio, sm_, src.size() * 8 - 1);
    pio_sm_exec(pio, sm_, pio_encode_jmp(qspi_offset_spi_rw));

    int timeout = default_timeout;
    while (tx_remain || rx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = *s++;
            --tx_remain;
        }

        if (rx_remain && !pio_sm_is_rx_fifo_empty(pio, sm_))
        {
            static_cast<void>(*rx);
            --rx_remain;
        }
        if (--timeout == 0) return false;
    }
    return true;
}

bool Qspi::qspi_read(DataType dest)
{
//    auto pio = get_pio(device_);
//    uint8_t* d = dest.data();
//    auto* rx = get_rx_fifo(pio, sm_);

//    std::size_t rx_remain = dest.size();

//    wait_until_previous_finished();

//    pio_sm_set_in_pins(pio, sm_, pin_base_);
//    pio_sm_put(pio, sm_, dest.size() * 2 - 2);
//    pio_sm_exec(pio, sm_, pio_encode_jmp(qspi_offset_qspi_r));

//    int timeout = default_timeout;
//    while (rx_remain)
//    {
//        if (!pio_sm_is_rx_fifo_empty(pio, sm_))
//        {
//            *d++ = *rx;
//            --rx_remain;
//        }
//        if (--timeout == 0) return false;
//    }
    return true;
}

bool Qspi::qspi_write(ConstDataType src)
{
    auto pio = get_pio(device_);
    const uint8_t* s = src.data();
    auto* tx = get_tx_fifo(pio, sm_);

    std::size_t tx_remain = src.size();

    wait_until_previous_finished();

    pio_sm_put(pio, sm_, src.size() * 2 - 1);
    pio_sm_exec(pio, sm_, pio_encode_jmp(qspi_offset_qspi_w));

    int timeout = default_timeout;
    while (tx_remain)
    {
        if (!pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = *s++;
            --tx_remain;
        }
        if (--timeout == 0) return false;
    }
    return true;
}

bool Qspi::qspi_command_read(ConstDataType command, DataType data, int wait_cycles)
{
    if (wait_cycles % 2 != 0) return false; 

    auto pio = get_pio(device_);

    auto* tx = get_tx_fifo(pio, sm_);
    auto* rx = get_rx_fifo(pio, sm_);
    wait_until_previous_finished();

    pio_sm_set_in_pins(pio, sm_, pin_base_);
    
    //pio_sm_put(pio, sm_, wait_cycles + command.size() * 2 - 1);
    //pio_sm_put(pio, sm_, data.size() * 2 - 1);

    pio_sm_put(pio, sm_, data.size() * 2 - 1);
    pio_sm_exec(pio, sm_, pio_encode_jmp(qspi_offset_qspi_command_r));

    int timeout = default_timeout;

    std::size_t command_remain = command.size();
    //uint8_t tmp[] = {0xeb, 0xab, 0xbc, 0xde};
    const uint8_t* s =  command.data();
    while (command_remain)
    {
        if (!pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = *s++;
            --command_remain;
        }
        //if (--timeout == 0) return false;
    }

    while (pio_sm_is_tx_fifo_full(pio, sm_)) {}
    pio_sm_put(pio, sm_, wait_cycles + 2);
    timeout = default_timeout;
    std::size_t wait_remain = wait_cycles / 2;

   // while (wait_remain)
   // {
   //     if (!pio_sm_is_rx_fifo_empty(pio, sm_))
   //     {
   //         static_cast<void>(*rx);
   //         --wait_remain;
   //     }
// //       if (!pio_sm_is_tx_fifo_full(pio, sm_))
// //       {
// //           *tx = 0xff; 
// //           --wait_remain;
// //       }

   //     //if (--timeout == 0) return false;
   // }

    timeout = default_timeout; 
    std::size_t data_remain = data.size();
    uint8_t* d = data.data();
    std::size_t drop_remain = 2;
//    while (drop_remain)
//    {
//        if (!pio_sm_is_rx_fifo_empty(pio, sm_))
//        {
//            static_cast<void>(*rx);
//            drop_remain--;
//        }
//    }

    while (data_remain)
    {
        if (!pio_sm_is_rx_fifo_empty(pio, sm_))
        {
            *d++ = *rx; 
            --data_remain;
        }
        //if (--timeout == 0) return false;
    }

    return true;
}

bool Qspi::qspi_command_write(ConstDataType command, ConstDataType data, int wait_cycles)
{
    if (wait_cycles % 2 != 0) return false; 

    auto pio = get_pio(device_);
    auto* tx = get_tx_fifo(pio, sm_);

    wait_until_previous_finished();

    int size = command.size() * 2 + wait_cycles + data.size() * 2 - 1;
    pio_sm_put(pio, sm_, size);

    pio_sm_exec(pio, sm_, pio_encode_jmp(qspi_offset_qspi_w));

    int timeout = default_timeout;

    std::size_t command_remain = command.size();
    const uint8_t* s = command.data();
    while (command_remain)
    {
        if (!pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = *s++;
            --command_remain;
        }
        if (--timeout == 0) return false;
    }

    timeout = default_timeout;
    std::size_t wait_remain = wait_cycles / 2;

    while (wait_remain)
    {
        if (!pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = 0;
            --wait_remain;
        }

        if (--timeout == 0) return false;
    }

    timeout = default_timeout; 
    std::size_t data_remain = data.size();
    const uint8_t* d = data.data();

    while (data_remain)
    {
        if (!pio_sm_is_tx_fifo_full(pio, sm_))
        {
            *tx = *d++;
            --data_remain;
        }
        if (--timeout == 0) return false;
    }

    return true;

}

bool Qspi::wait_until_previous_finished()
{
    const int idle_wait_start = qspi_offset_idle_wait_loop;
    const int idle_wait_end = qspi_offset_idle_wait_end;
    auto pio = get_pio(device_); 
    int timeout = 10000;
    while (pio->sm[sm_].addr < idle_wait_start
        || pio->sm[sm_].addr >= idle_wait_end) 
    {
        //if (--timeout == 0) return false;
    }

    return true;
}
