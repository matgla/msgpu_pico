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

#include <filesystem>
#include <memory>

#include <sys/mman.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "ips6404/ips6404.hpp"
#include "qspi_bus.hpp"
#include <termios.h>

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
    close(serial_write_port);

    if (std::filesystem::exists("/tmp/gpu_com"))
    {
        std::filesystem::remove("/tmp/gpu_com");
    }
    if (std::filesystem::exists("/tmp/gpu_com_2"))
    {
        std::filesystem::remove("/tmp/gpu_com_2");
    }
    exit(0);
}

namespace msgpu
{

void initialize_application_specific()
{
    constexpr const char *in_file = "/tmp/gpu_com";
    if (std::filesystem::exists(in_file))
    {
        std::filesystem::remove(in_file);
    }

    constexpr const char *out_file = "/tmp/gpu_com_2";
    if (std::filesystem::exists(out_file))
    {
        std::filesystem::remove(out_file);
    }

    mkfifo(in_file, 0666);
    mkfifo(out_file, 0666);

    printf("Opening input: /tmp/gpu_com\n");
    serial_port_id = open(in_file, O_RDONLY);
    printf("Opening output: /tmp/gpu_com_2\n");
    serial_write_port = open(out_file, O_WRONLY);

    signal(SIGINT, exit_handler);
}

uint8_t read_byte()
{
    uint8_t byte;
    if (read(serial_port_id, &byte, sizeof(byte)) == -1)
    {
        printf("Error while reading\n");
    }
    return byte;
}

void write_bytes(std::span<const uint8_t> data)
{
    write(serial_write_port, data.data(), data.size());
}

void write_bytes(const void *data, std::size_t size)
{
    write(serial_write_port, data, size);
}

} // namespace msgpu
