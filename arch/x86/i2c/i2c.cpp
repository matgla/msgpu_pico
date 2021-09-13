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

#include <map>
#include <string> 

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>


namespace msgpu 
{

namespace 
{

struct MemInfo 
{
    int fd; 
    std::string name;
    sem_t* sem;
};

std::map<uint8_t, MemInfo> slave_fd_map;
MemInfo master_fd;

MemInfo *my_fd;

constexpr const char* fifo_name = "i2c_bus";
}

I2C::I2C(uint8_t slave_address, uint32_t pin_scl, uint32_t pin_sda)
{
    //std::string name = std::string("i2c_") + std::to_string(slave_address) + std::string("_slave");

    //mkfifo(fifo_name, 0666);

//    slave_fd_map[slave_address] = MemInfo {
//        .fd = fd,
//        .name = name
//    };

//    my_fd = &slave_fd_map[slave_address];

}

I2C::I2C(uint32_t pin_scl, uint32_t pin_sda)
{
    mkfifo(fifo_name, 0666);
//    std::string master_name = "i2c_master";

//    master_fd = {
//        .fd = m_fd, 
//        .name = master_name,
//    };

//    my_fd = &master_fd;
}

I2C::~I2C()
{
    
}

void I2C::read(DataType data)
{
//    if (!sem_wait(my_fd->sem))
//    {
    // printf("I2C read\n");
    int fd = open(fifo_name, O_RDONLY);
    ::read(fd, data.data(), data.size());
    close(fd);
//        sem_post(my_fd->sem);
//    }
}

void I2C::write(ConstDataType data)
{
    // printf("I2C write\n");
//    if (!sem_wait(my_fd->sem))
//    {
    int fd = open(fifo_name, O_WRONLY);
    ::write(fd, data.data(), data.size());
    close(fd);
//        sem_post(my_fd->sem);
//    }

}

void I2C::read(uint8_t address, DataType data)
{
    // printf("I2C read from slave: 0x%x\n", address);
    int fd = open(fifo_name, O_RDONLY);
    ::read(fd, data.data(), data.size());
    close(fd);
}

void I2C::write(uint8_t address, ConstDataType data)
{
    // printf("I2C write to slave: 0x%x\n", address);
    int fd = open(fifo_name, O_WRONLY);
    ::write(fd, data.data(), data.size());
    close(fd);
        //sem_post(my_fd->sem);
    //}
   
}

} // namespace msgpu
