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

#include "hardware/i2c.h"
#include "hardware/gpio.h"

namespace msgpu 
{

I2C::I2C(uint8_t slave_address, uint32_t pin_scl, uint32_t pin_sda)
    : I2C(pin_scl, pin_sda)
{
   i2c_set_slave_mode(i2c0, true, slave_address);
}

I2C::I2C(uint32_t pin_scl, uint32_t pin_sda)
{
    i2c_init(i2c0, 1000);
    gpio_set_function(pin_scl, GPIO_FUNC_I2C);
    gpio_set_function(pin_sda, GPIO_FUNC_I2C);
    gpio_pull_up(pin_scl);
    gpio_pull_up(pin_sda);
}

void I2C::read(DataType data)
{
    i2c_read_raw_blocking(i2c0, data.data(), data.size());
}

void I2C::write(ConstDataType data)
{
    i2c_write_raw_blocking(i2c0, data.data(), data.size());
}

void I2C::read(uint8_t address, DataType data)
{
    i2c_read_blocking(i2c0, address, data.data(), data.size(), false);
}

void I2C::write(uint8_t address, ConstDataType data)
{
    i2c_write_blocking(i2c0, address, data.data(), data.size(), false);
}


} // namespace msgpu
