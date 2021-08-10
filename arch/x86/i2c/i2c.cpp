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
}

I2C::I2C(uint8_t slave_address, uint32_t pin_scl, uint32_t pin_sda)
{
    std::string name = std::string("i2c_") + std::to_string(slave_address) + std::string("_slave");

    std::string semaphore_name = std::string("/") + name;

    sem_t *sem = sem_open(semaphore_name.c_str(), O_CREAT, 0666);
    mkfifo(name.c_str(), 0666);
    int fd = open(name.c_str(), O_RDWR);

    slave_fd_map[slave_address] = MemInfo {
        .fd = fd,
        .name = name
    };

    my_fd = &slave_fd_map[slave_address];

}

I2C::I2C(uint32_t pin_scl, uint32_t pin_sda)
{
    std::string master_name = "i2c_master";
    int m_fd = shm_open(master_name.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);

    ftruncate(m_fd, 256);
    void* master_ptr = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    sem_t* master_sem = sem_open(master_name.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 0); 

    master_fd = {
        .fd = m_fd, 
        .name = master_name,
        .ptr = master_ptr,
        .sem = master_sem
    };

    my_fd = &master_fd;
}

I2C::~I2C()
{
    shm_unlink(master_fd.name.c_str());
    munmap(master_fd.ptr, 256);
    sem_close(master_fd.sem);
    for (const auto &[key, value] : slave_fd_map)
    {
        shm_unlink(value.name.c_str());
        munmap(value.ptr, 256);
        sem_close(value.sem);
    }
}

void I2C::read(DataType data)
{
    if (!sem_wait(my_fd->sem))
    {
        printf("I2C semaphore released\n");
        std::memcpy(
        sem_post(my_fd->sem);
    }
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
