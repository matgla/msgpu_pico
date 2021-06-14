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

#include <cstdint>

#include <span>

class Qspi 
{
public:
    static void init();
    using DataType = std::span<uint8_t>;
    using ConstDataType = std::span<const uint8_t>;

    enum Device {
        Ram, 
        IO
    };

    enum Mode {
        SPI,
        QSPI_write, 
        QSPI_read 
    };
    static void switch_to(Mode m);
    static void chip_select(Device d, bool select);

    static int read8_write8_blocking(DataType write_buffer, ConstDataType read_buffer);
    static int spi_read8(DataType write_buffer);
    static int spi_write8(ConstDataType read_buffer);
    static int qspi_read8(DataType write_buffer);
    static int qspi_write8(ConstDataType read_buffer);

    static void switch_to_qspi_read();
};

