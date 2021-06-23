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

QspiPSRAM::QspiPSRAM(Qspi& qspi)
    : qspi_(qspi)
    , qspi_mode_(false)
{
}

bool QspiPSRAM::init()
{
    printf("Now under qspi mode\n");
    if (!reset())
    {
        printf("Reset failure\n");
        qspi_mode_ = true;
        exit_qpi_mode();
    }
    else 
    {
        return true; 
    }
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
    msgpu::sleep_us(200); // Wait for initialization from datasheet 

    constexpr uint8_t reset_enable_cmd[] = {0x66};
    constexpr uint8_t reset_cmd[] = {0x99};
    qspi_.spi_write(reset_enable_cmd);
    qspi_.spi_write(reset_cmd);

    msgpu::sleep_us(100);
    return perform_post();
}

std::size_t QspiPSRAM::write(std::size_t address, const ConstDataBuffer data)
{
    const uint8_t write_cmd[] = {0x38, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff};
    qspi_.qspi_command_write(write_cmd, data, 0);
    return data.size();
}

std::size_t QspiPSRAM::read(const std::size_t address, DataBuffer data)
{
    const uint8_t read_cmd[] = {0xeb, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff};

    qspi_.qspi_command_read(read_cmd, data, 6);
    return data.size();
}

bool QspiPSRAM::perform_post()
{
    constexpr uint8_t read_eid_cmd[] = {0x9f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t eid_buffer[sizeof(read_eid_cmd)] = {};
    qspi_.spi_transmit(read_eid_cmd, eid_buffer);

    printf ("Readed EID: { ");
    for (auto b : eid_buffer)
    {
        printf("0x%x, ", b);
    }
    printf(" }\n");
    return eid_buffer[4] == 0x0d && eid_buffer[5] == 0x5d;
}

void QspiPSRAM::exit_qpi_mode()
{
    if (!qspi_mode_) return;
    qspi_mode_ = false;
    constexpr uint8_t exit_qpi_cmd[] = {0xf5};
    qspi_.qspi_write(exit_qpi_cmd);
}

void QspiPSRAM::enter_qpi_mode()
{
    if (qspi_mode_) return;
    qspi_mode_ = true;
    constexpr uint8_t enter_qpi_cmd[] = {0x35};
    qspi_.spi_write(enter_qpi_cmd);
}

} // namespace msgpu::memory
