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

#pragma once 

#include <cstddef>
#include <cstdint>

#include "hardware/pio.h" 

#include "qspi.pio.h"

struct pio_qspi_inst
{
    PIO pio;
    uint32_t sm;
    uint32_t cs_pin;
};

void pio_spi_write8_blocking(const pio_qspi_inst* spi, const uint8_t* src, std::size_t len, uint32_t pin_cs);

void pio_spi_read8_blocking(const pio_qspi_inst* spi, uint8_t* dst, std::size_t len);

void pio_spi_write8_read8_blocking(const pio_qspi_inst* spi, const uint8_t* src, uint8_t* dst, std::size_t len);

void pio_qspi_write8_blocking(const pio_qspi_inst* spi, const uint8_t* src, std::size_t len);

void pio_qspi_read8_blocking(const pio_qspi_inst* spi, uint8_t* dst, std::size_t len);

void pio_qspi_write8_read8_blocking(const pio_qspi_inst* spi, const uint8_t* src, uint8_t* dst, std::size_t len);

void pio_qspi_spi_command_rw(const pio_qspi_inst* spi, const uint8_t* command, std::size_t command_size, const uint8_t* data, uint8_t* readed, std::size_t data_size, int wait_cycles);




