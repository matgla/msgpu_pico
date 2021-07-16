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

#include "arch/i2c.hpp"

namespace msgpu 
{

I2C::I2C(uint8_t slave_address, uint32_t pin_scl, uint32_t pin_sda)
{

}

I2C::I2C(uint32_t pin_scl, uint32_t pin_sda)
{

}

void I2C::read(DataType data)
{
}

void I2C::write(ConstDataType data)
{
}

void I2C::read(uint8_t address, DataType data)
{
}

void I2C::write(uint8_t address, ConstDataType data)
{
}
} // namespace msgpu
