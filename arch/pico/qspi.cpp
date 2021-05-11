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

#include "qspi.hpp"

#include "qspi/pio_qspi.h"

constexpr uint32_t pin_sck = 18;
constexpr uint32_t pin_mosi = 19;
constexpr uint32_t pin_miso = 19; // for testing purposes this is loopback
constexpr uint32_t pin_io2 = 20;
constexpr uint32_t pin_io3 = 20;

static pio_qspi_inst qspi = {
    .pio = pio1,
    .sm = 0
};

void Qspi::init() 
{
    float clkdiv = 31.25f;

    uint32_t cpha0_prog_offs = pio_add_program(qspi.pio, &qspi_cpha0_program);
    uint32_t cpha1_prog_offs = pio_add_program(qspi.pio, &qspi_cpha1_program);

    pio_qspi_init(qspi.pio, qspi.sm, 
        cpha0_prog_offs,
        8,
        clkdiv, 
        0, 
        0, 
        pin_sck, 
        pin_mosi, 
        pin_miso,
        pin_io2,
        pin_io3
    );
}

void Qspi::chip_select(Device d)
{

}

void Qspi::switch_to(Mode m)
{
    switch (m)
    {
        case Mode::SPI: 
        {
        } break;
        case Mode::QSPI_write: 
        {
        } break;
        case Mode::QSPI_read:
        {
            pio_qspi_init_qspi_read(

                    );
        } break;
    }
}

int Qspi::read8_write8_blocking(DataType write_buffer, ConstDataType read_buffer)
{
    pio_spi_write8_read8_blocking(&qspi, read_buffer.data(), write_buffer.data(), write_buffer.size());
    return write_buffer.size();
}

int Qspi::spi_read8(DataType write_buffer)
{
    pio_spi_read8_blocking(&qspi, write_buffer.data(), write_buffer.size());
    return write_buffer.size();
}

int Qspi::spi_write8(ConstDataType read_buffer)
{
    pio_spi_write8_blocking(&qspi, read_buffer.data(), read_buffer.size());
    return read_buffer.size();
}

int Qspi::qspi_read8(DataType write_buffer)
{
    pio_qspi_read8_blocking(&qspi, write_buffer.data(), write_buffer.size());
    return write_buffer.size();
}

int Qspi::qspi_write8(ConstDataType read_buffer)
{
    pio_qspi_write8_blocking(&qspi, read_buffer.data(), read_buffer.size());
    return read_buffer.size();
}

