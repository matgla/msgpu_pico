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

#include <sys/mman.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <termios.h>
#include "qspi_bus.hpp"
#include "ips6404/ips6404.hpp"

#include "hal_dma.hpp"

namespace 
{

static int serial_port_id;
static int serial_write_port;

} // namespace 


void exit_handler(int sig)
{
    static_cast<void>(sig);
     
    hal::close_usart();
    close(serial_port_id);
    exit(0);
}

namespace msgpu 
{

void initialize_application_specific()
{
    printf("Opening serial port: /tmp/msgpu_virtual_serial_0\n");
    mkfifo("/tmp/gpu_com", 0666);
    mkfifo("/tmp/gpu_com_2", 0666);

    printf("Opening input: /tmp/gpu_com\n");
    serial_port_id = open("/tmp/gpu_com", O_RDONLY);
    printf("Opening output: /tmp/gpu_com_2\n");
    serial_write_port = open("/tmp/gpu_com_2", O_WRONLY);
    signal(SIGINT, exit_handler);
}

uint8_t read_byte() 
{
    uint8_t byte; 
    read(serial_port_id, &byte, sizeof(byte));
    return byte;
}

void write_bytes(std::span<const uint8_t> data)
{
    write(serial_write_port, data.data(), data.size());
}

void write_bytes(const void* data, std::size_t size)
{
    write(serial_write_port, data, size);
}

} // namespace msgpu


