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

namespace msgpu 
{

class I2C
{
public:
    using DataType = std::span<uint8_t>;
    using ConstDataType = std::span<uint8_t>;
    I2C(uint8_t slave_address, uint32_t pin_scl, uint32_t pin_sda);
    I2C(uint32_t pin_scl, uint32_t pin_sda);
    ~I2C();

    void read(DataType data);
    void write(ConstDataType data);

    void read(uint8_t address, DataType data);
    void write(uint8_t address, ConstDataType data);
};

} // namespace msgpu

