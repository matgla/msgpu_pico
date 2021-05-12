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

#include "pio_qspi.h"

#include <cstdio>

void __time_critical_func(pio_spi_write8_read8_blocking)(const pio_qspi_inst* qspi, const uint8_t* src, uint8_t* dst, std::size_t len)
{
    std::size_t tx_remain = len, rx_remain = len; 
    io_rw_8* txfifo = (io_rw_8*)&qspi->pio->txf[qspi->sm];
    io_rw_8* rxfifo = (io_rw_8*) &qspi->pio->rxf[qspi->sm];

    while (tx_remain || rx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(qspi->pio, qspi->sm)) 
        {
            *txfifo = *src++;
            --tx_remain;
        }
        if (rx_remain && !pio_sm_is_rx_fifo_empty(qspi->pio, qspi->sm)) 
        {
            *dst++ = *rxfifo;
            --rx_remain;
        }
    }
}

void __time_critical_func(pio_spi_read8_blocking)(const pio_qspi_inst* qspi, uint8_t* dst, std::size_t len) 
{
    std::size_t rx_remain = len; 
    io_rw_8* rxfifo = (io_rw_8*) &qspi->pio->rxf[qspi->sm];

    while (rx_remain)
    {
        if (rx_remain && !pio_sm_is_rx_fifo_empty(qspi->pio, qspi->sm)) 
        {
            *dst++ = *rxfifo;
            --rx_remain;
        }
    }

}

void __time_critical_func(pio_spi_write8_blocking)(const pio_qspi_inst* qspi, const uint8_t* src, std::size_t len)
{
    std::size_t tx_remain = len; 
    io_rw_8* txfifo = (io_rw_8*)&qspi->pio->txf[qspi->sm];

    while (tx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(qspi->pio, qspi->sm)) 
        {
            *txfifo = *src++;
            --tx_remain;
        }
    }
}

void __time_critical_func(pio_qspi_read8_blocking)(const pio_qspi_inst* qspi, uint8_t* dst, std::size_t len) 
{
    std::size_t rx_remain = len; 
    io_rw_8* rxfifo = (io_rw_8*) &qspi->pio->rxf[qspi->sm];

    while (rx_remain)
    {
        if (rx_remain && !pio_sm_is_rx_fifo_empty(qspi->pio, qspi->sm)) 
        {
            *dst++ = *rxfifo;
            --rx_remain;
        }
    }

}

void __time_critical_func(pio_qspi_write8_blocking)(const pio_qspi_inst* qspi, const uint8_t* src, std::size_t len)
{
    std::size_t tx_remain = len; 
    io_rw_8* txfifo = (io_rw_8*)&qspi->pio->txf[qspi->sm];

    while (tx_remain)
    {
        if (tx_remain && !pio_sm_is_tx_fifo_full(qspi->pio, qspi->sm)) 
        {
            *txfifo = *src++;
            --tx_remain;
        }
    }
}



