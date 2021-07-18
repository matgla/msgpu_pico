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

#include "ips6404/ips6404.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

namespace msgpu 
{
namespace stubs 
{

struct shared_memory 
{
    sem_t sync;
    char buf[
        8 // MB 
    * 1024
    * 1024
    ];
};

IPS6404Stub::IPS6404Stub(std::string_view name)
    : memory_fd_(0)
    , buffer_name_(name)
{
    printf("Allocating shared memory for: %s\n", name.data());

    memory_fd_ = shm_open(name.data(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);

}

IPS6404Stub::~IPS6404Stub()
{
    shm_unlink(buffer_name_.data());
}

void IPS6404Stub::init() 
{
}

void IPS6404Stub::read(const DataType &buf, std::size_t len)
{

}

void IPS6404Stub::write(const ConstDataType &buf, std::size_t len)
{

}

void IPS6404Stub::transmit(const ConstDataType &src, const DataType &dest, std::size_t write_len, std::size_t read_len)
{
    printf("Transmit\n");
    for (const auto byte : src) 
    {
        printf("0x%x, ", byte);
    }
    printf("\n");
    
}

} // namespace stubs
} // namespace msgpu
