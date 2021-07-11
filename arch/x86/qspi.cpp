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

#include "qspi.hpp"

namespace msgpu 
{

Qspi::Qspi(const QspiConfig device, float clkdiv)
    : config_(device)
    , clkdiv_(clkdiv)
{
}

void Qspi::init()
{
}

void Qspi::init_pins()
{
}

bool Qspi::spi_transmit(ConstDataType src, DataType dest)
{
    static_cast<void>(src);
    static_cast<void>(dest);
    return true;
}

bool Qspi::spi_read(DataType dest)
{
    static_cast<void>(dest);
    return true;
}
    
bool Qspi::spi_write(ConstDataType src)
{
    static_cast<void>(src);
    return true;
}

bool Qspi::qspi_read(DataType dest)
{
    static_cast<void>(dest);
    return true;
}

bool Qspi::qspi_write(ConstDataType src)
{
    static_cast<void>(src);
    return true;
}

bool Qspi::qspi_command_read(DataType command, DataType data)
{
    static_cast<void>(command);
    static_cast<void>(data);
    return true;
}


bool Qspi::qspi_command_write(ConstDataType command, ConstDataType data)
{
    static_cast<void>(command);
    static_cast<void>(data);
    return true;
}

void Qspi::wait_for_finish() const
{
}

void Qspi::acquire_bus() const
{
}

void Qspi::release_bus() const
{
}

} // namespace msgpu
