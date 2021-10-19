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

#include <filesystem>
#include <map>
#include <string>

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace msgpu
{

namespace
{

struct MemInfo
{
    int fd;
    std::string name;
    sem_t *sem;
};

std::map<uint8_t, MemInfo> slave_fd_map;
MemInfo master_fd;

MemInfo *my_fd;

static int r_fd;
static int w_fd;

constexpr const char *read_fifo_name  = "i2c_bus_r";
constexpr const char *write_fifo_name = "i2c_bus_w";
} // namespace

I2C::I2C(uint8_t slave_address, uint32_t pin_scl, uint32_t pin_sda)
{
    // std::string name = std::string("i2c_") + std::to_string(slave_address) +
    // std::string("_slave");

    // mkfifo(fifo_name, 0666);

    //    slave_fd_map[slave_address] = MemInfo {
    //        .fd = fd,
    //        .name = name
    //    };

    //    my_fd = &slave_fd_map[slave_address];
    //

    r_fd = open(write_fifo_name, O_RDONLY);
    w_fd = open(read_fifo_name, O_WRONLY);
}

I2C::I2C(uint32_t pin_scl, uint32_t pin_sda)
{
    if (std::filesystem::exists(read_fifo_name))
    {
        std::filesystem::remove(read_fifo_name);
    }
    if (std::filesystem::exists(write_fifo_name))
    {
        std::filesystem::remove(write_fifo_name);
    }
    mkfifo(read_fifo_name, 0666);
    mkfifo(write_fifo_name, 0666);
    //    std::string master_name = "i2c_master";

    //    master_fd = {
    //        .fd = m_fd,
    //        .name = master_name,
    //    };

    //    my_fd = &master_fd;
    printf("I2C STUB, open file: %s\n", write_fifo_name);
    w_fd = open(write_fifo_name, O_WRONLY);
    printf("I2C STUB, open file: %s\n", read_fifo_name);
    r_fd = open(read_fifo_name, O_RDONLY);
}

I2C::~I2C()
{
    close(w_fd);
    close(r_fd);
}

void I2C::read(DataType data)
{
    //    if (!sem_wait(my_fd->sem))
    //    {
    // printf("I2C read\n");
    ::read(r_fd, data.data(), data.size());
    //        sem_post(my_fd->sem);
    //    }
}

void I2C::write(ConstDataType data)
{
    // printf("I2C write\n");
    //    if (!sem_wait(my_fd->sem))
    //    {
    ::write(w_fd, data.data(), data.size());
    //        sem_post(my_fd->sem);
    //    }
}

void I2C::read(uint8_t address, DataType data)
{
    // printf("I2C read from slave: 0x%x\n", address);
    ::read(r_fd, data.data(), data.size());
}

void I2C::write(uint8_t address, ConstDataType data)
{
    // printf("I2C write to slave: 0x%x\n", address);
    ::write(w_fd, data.data(), data.size());
    // sem_post(my_fd->sem);
    //}
}

} // namespace msgpu
