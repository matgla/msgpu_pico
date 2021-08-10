// This file is part of msgput project.
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

#include <thread>
#include <memory>

#include <unistd.h>
#include <termios.h> 
#include <fcntl.h>

#include "qspi_bus.hpp"
#include "ips6404/ips6404.hpp"


namespace msgpu 
{
namespace 
{

static int serial_port_id;

}

void initialize_board()
{

    serial_port_id = open("/tmp/msgpu_virtual_serial_0", O_RDWR);

    msgpu::QspiBus::get().register_device(0, 
        std::make_unique<msgpu::stubs::IPS6404Stub>("qspi_framebuffer_out"));

    initialize_application_specific();
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

void sleep_ms(uint32_t time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

uint32_t get_millis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64_t get_us() 
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void sleep_us(uint32_t time)
{
    std::this_thread::sleep_for(std::chrono::microseconds(time));
}

} // namespace msgpu 

