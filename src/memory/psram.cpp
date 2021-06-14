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

#include "memory/psram.hpp"

#include <cstdio>

#include "board.hpp"

namespace msgpu::memory 
{

QspiPSRAM::QspiPSRAM(Qspi::Device device)
    : device_(device)
{
}

bool QspiPSRAM::init()
{
    qspi_.init();
    qspi_.switch_to(Qspi::Mode::SPI);
    printf("Now under qspi mode\n");
    if (!reset())
    {
        printf("Reset failure\n");
        exit_qpi_mode();
    }
    qspi_.switch_to(Qspi::Mode::SPI);
    if (reset())
    {
        enter_qpi_mode();
        return true;
    }
    return false;
}

bool QspiPSRAM::reset()
{
    printf("Reset\n");
    qspi_.chip_select(device_, false);
    msgpu::sleep_us(200); // Wait for initialization from datasheet 

    constexpr uint8_t reset_enable_cmd[] = {0x66};
    constexpr uint8_t reset_cmd[] = {0x99};

    qspi_.chip_select(device_, true);
    qspi_.spi_write8(reset_enable_cmd);
    qspi_.chip_select(device_, false);
    qspi_.chip_select(device_, true);
    qspi_.spi_write8(reset_cmd);
    qspi_.chip_select(device_, false);

    msgpu::sleep_us(100);
    return perform_post();
}

std::size_t QspiPSRAM::write(const std::size_t address, const ConstDataBuffer data)
{
    qspi_.switch_to(Qspi::Mode::QSPI_write);
    const uint8_t write_cmd[] = {0x38, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff};
    qspi_.chip_select(device_, true);
    qspi_.qspi_write8(write_cmd);
    qspi_.qspi_write8(data);
    qspi_.chip_select(device_, false);
    return data.size();
//    exit_qpi_mode();

//    qspi_.switch_to(Qspi::Mode::SPI);
//    const uint8_t write_cmd[] = {0x02, (address >> 16) & 0xff, (address > 8) & 0xff, address & 0xff};
//    qspi_.chip_select(device_, true);
//    qspi_.spi_write8(write_cmd);
//    qspi_.spi_write8(data);
//    qspi_.chip_select(device_, false);

//    enter_qpi_mode();
//    return 0;
}

std::size_t QspiPSRAM::read(const std::size_t address, DataBuffer data)
{
    qspi_.switch_to(Qspi::Mode::QSPI_write);
    const uint8_t read_cmd[] = {0xeb, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff};

    uint8_t wait_cycles[3];
    qspi_.chip_select(device_, true);
    qspi_.qspi_write8(read_cmd);
    //qspi_.switch_to(Qspi::Mode::QSPI_read);
    qspi_.switch_to_qspi_read();
    qspi_.qspi_read8(wait_cycles);
    qspi_.qspi_read8(data);
    //msgpu::sleep_us(1);
    qspi_.chip_select(device_, false);
    return data.size();
}

bool QspiPSRAM::perform_post()
{
    constexpr uint8_t read_eid_cmd[] = {0x9f, 0x00, 0x00, 0x00};
    uint8_t eid_buffer[8] = {};
    qspi_.chip_select(device_, true);
    qspi_.spi_write8(read_eid_cmd);
    qspi_.spi_read8(eid_buffer);
    qspi_.chip_select(device_, false);

    printf ("Readed EID: { ");
    for (auto b : eid_buffer)
    {
        printf("0x%x, ", b);
    }
    printf(" }\n");
    return eid_buffer[0] == 0x0d && eid_buffer[1] == 0x5d;
}

void QspiPSRAM::exit_qpi_mode()
{
    constexpr uint8_t exit_qpi_cmd[] = {0xf5};
    qspi_.switch_to(Qspi::Mode::QSPI_write);
    qspi_.chip_select(device_, true);
    qspi_.qspi_write8(exit_qpi_cmd);
    qspi_.chip_select(device_, false);
    qspi_.switch_to(Qspi::Mode::SPI);
}

void QspiPSRAM::enter_qpi_mode()
{
    constexpr uint8_t enter_qpi_cmd[] = {0x35};
    qspi_.chip_select(device_, true);
    qspi_.spi_write8(enter_qpi_cmd);
    qspi_.chip_select(device_, false);
    qspi_.switch_to(Qspi::Mode::QSPI_write);
}

} // namespace msgpu::memory
