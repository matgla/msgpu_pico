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
    enum class Device : uint8_t 
    {
        framebuffer
    };

    Qspi(const Device device, float clkdiv);

    void init();
    
    using DataType = std::span<uint8_t>;
    using ConstDataType = std::span<const uint8_t>;

    bool spi_transmit(ConstDataType src, DataType dest);
    bool spi_read(DataType dest);
    bool spi_write(ConstDataType src);

    bool qspi_read(DataType dest);
    bool qspi_write(ConstDataType src);

    bool qspi_command_read(ConstDataType command, DataType data, int wait_cycles);

    bool qspi_command_write(ConstDataType command, ConstDataType data, int wait_cycles);

    void wait_for_finish() const;

private:
    void setup_dma_write(ConstDataType src, int channel, int chain_to = -1);
    void setup_dma_read(DataType dest, int channel, int chain_to = -1);
    
    void setup_dma_command_write(ConstDataType cmd, ConstDataType data);
    void setup_dma_command_read(ConstDataType cmd, DataType data); 

    bool wait_until_previous_finished();
    const Device device_;
    const uint32_t pin_sck_;
    const uint32_t pin_base_;
    const uint32_t pin_cs_;
    const uint32_t sm_;
    const float clkdiv_;
};

