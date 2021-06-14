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
constexpr uint32_t pin_miso = 20; // for testing purposes this is loopback
constexpr uint32_t pin_io2 = 21;
constexpr uint32_t pin_io3 = 22;
constexpr uint32_t pin_cs = 26;

static pio_qspi_inst spi = {
    .pio = pio1,
    .sm = 0
};

static pio_qspi_inst qspi_read = {
    .pio = pio1,
    .sm = 1
};

static pio_qspi_inst qspi_write = {
    .pio = pio1,
    .sm = 2 
};

static uint32_t cpha0_prog_offs;
static uint32_t cpha1_prog_offs;
static uint32_t qspi_write_prog_offs;
static uint32_t qspi_read_prog_offs;
float clkdiv = 125.0f;

void Qspi::init() 
{

    cpha0_prog_offs = pio_add_program(spi.pio, &spi_cpha0_program);
    
    qspi_write_prog_offs = pio_add_program(qspi_write.pio, &qspi_write_program);
    qspi_read_prog_offs = pio_add_program(qspi_read.pio, &qspi_read_program);

    gpio_init(pin_cs);
    gpio_put(pin_cs, 1);
    gpio_set_dir(pin_cs, GPIO_OUT);

    pio_qspi_spi_init(spi.pio, 
        spi.sm, 
        cpha0_prog_offs,
        8, 
        clkdiv, 
        pin_sck,
        pin_mosi,
        pin_miso
    );

    pio_qspi_qspi_read_init(qspi_read.pio,
        qspi_read.sm,
        qspi_read_prog_offs,
        8,
        clkdiv,
        pin_sck,
        pin_mosi 
    );

    pio_qspi_qspi_write_init(qspi_write.pio,
        qspi_write.sm,
        qspi_write_prog_offs,
        8,
        clkdiv,
        pin_sck,
        pin_mosi
    );

    pio_qspi_set_spi(spi.pio, spi.sm, pin_sck, pin_mosi, pin_miso);

}

void __time_critical_func(Qspi::chip_select)(Device d, bool select)
{
    switch (d)
    {
        case Device::Ram:
        {
            if (select)
            {
                gpio_put(pin_cs, 0);
            }
            else 
            {
                gpio_put(pin_cs, 1);
            }
        } break;
        case Device::IO:
        {
        } break;
    }
}

void Qspi::switch_to(Mode m)
{
    switch (m)
    {
        case Mode::SPI: 
        {
            pio_qspi_disable(qspi_read.pio, qspi_read.sm);
            pio_qspi_disable(qspi_write.pio, qspi_write.sm);
            pio_qspi_set_spi(spi.pio, spi.sm, pin_sck, pin_mosi, pin_miso);
        } break;
        case Mode::QSPI_write: 
        {
            pio_qspi_disable(qspi_read.pio, qspi_read.sm);
            pio_qspi_disable(spi.pio, spi.sm);
            pio_qspi_set_qspi_write(qspi_write.pio, qspi_write.sm, pin_sck, pin_mosi);
        } break;
        case Mode::QSPI_read:
        {
            pio_qspi_disable(qspi_write.pio, qspi_write.sm);
            //pio_qspi_disable(spi.pio, spi.sm);
            pio_qspi_set_qspi_read(qspi_read.pio, qspi_read.sm, pin_sck, pin_mosi);
        } break;
    }
}

void __time_critical_func(Qspi::switch_to_qspi_read)()
{
    //pio_qspi_disable(qspi_write.pio, qspi_write.sm);
    pio_qspi_set_qspi_read(qspi_read.pio, qspi_read.sm, pin_sck, pin_mosi);
}

int Qspi::read8_write8_blocking(DataType write_buffer, ConstDataType read_buffer)
{
    pio_spi_write8_read8_blocking(&spi, read_buffer.data(), write_buffer.data(), write_buffer.size());
    return write_buffer.size();
}

int Qspi::spi_read8(DataType write_buffer)
{
    pio_spi_read8_blocking(&spi, write_buffer.data(), write_buffer.size());
    return write_buffer.size();
}

int Qspi::spi_write8(ConstDataType read_buffer)
{
    pio_spi_write8_blocking(&spi, read_buffer.data(), read_buffer.size());
    return read_buffer.size();
}

int Qspi::qspi_read8(DataType write_buffer)
{
    pio_qspi_read8_blocking(&qspi_read, write_buffer.data(), write_buffer.size());
    return write_buffer.size();
}

int Qspi::qspi_write8(ConstDataType read_buffer)
{
    pio_qspi_write8_blocking(&qspi_write, read_buffer.data(), read_buffer.size());
    return read_buffer.size();
}

