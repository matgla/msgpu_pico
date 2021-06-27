// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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

#include "interfaces/spi.hpp"

#include <cstring>

#include "interfaces/usart.hpp"

namespace
{
    uint8_t rx_buffer_[100];
}

void Spi::initialize()
{
}

void Spi::write(const DataStream& data)
{
}

void Spi::write(uint8_t data)
{
}

uint8_t Spi::read()
{
    return {};
}
