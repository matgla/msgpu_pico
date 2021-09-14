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

#include "board.hpp"

#include <memory>

#include <unistd.h>
#include <fcntl.h>

#include "qspi_bus.hpp"
#include "ips6404/ips6404.hpp"


namespace msgpu 
{

namespace 
{

static int serial_port_id;

} // namespace 


void initialize_application_specific()
{
    printf("Opening serial port: /tmp/msgpu_virtual_serial_0\n");
    serial_port_id = open("/tmp/msgpu_virtual_serial_0", O_RDWR);
}

uint8_t read_byte() 
{
    uint8_t byte; 
    read(serial_port_id, &byte, sizeof(byte));
    return byte;
}

void write_bytes(std::span<const uint8_t> data)
{
    write(serial_port_id, data.data(), data.size());
}

void write_bytes(const void* data, std::size_t size)
{
    write(serial_port_id, data, size);
}

} // namespace msgpu


