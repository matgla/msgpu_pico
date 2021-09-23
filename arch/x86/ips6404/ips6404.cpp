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

#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "panic.hpp"

namespace msgpu
{
namespace stubs
{

namespace
{
constexpr int address_offset = 3;
constexpr int command_offset = 1;

} // namespace

IPS6404StubSm::IPS6404StubSm(SharedMemory &memory)
    : memory_(memory)
{
}

void IPS6404StubSm::init()
{
    printf("Initialize IPS6404 StateMachine\n");
    sem_post(memory_.sem);
}

void IPS6404StubSm::read_base(const evTransmit &ev, int wait_cycles)
{
    const std::size_t address = ev.src[1] << 16 | ev.src[2] << 8 | ev.src[3];
    const std::size_t offset  = command_offset + address_offset + wait_cycles;
    sem_wait(memory_.sem);

    std::memcpy(ev.dest.data(), &memory_.memory[address], ev.dest.size());

    sem_post(memory_.sem);
}

void IPS6404StubSm::read(const evTransmit &ev)
{
    read_base(ev, 0);
}

void IPS6404StubSm::spi_fast_read(const evTransmit &ev)
{
    read_base(ev, 1);
}

void IPS6404StubSm::qpi_fast_read(const evTransmit &ev)
{
    read_base(ev, 3);
}

void IPS6404StubSm::write(const evTransmit &ev)
{
    const std::size_t address = ev.src[1] << 16 | ev.src[2] << 8 | ev.src[3];
    const std::size_t offset  = command_offset + address_offset;

    sem_wait(memory_.sem);

    std::memcpy(&memory_.memory[address], &ev.src[offset], ev.src.size() - offset);
    sem_post(memory_.sem);
}

void IPS6404StubSm::switch_to_quad()
{
    printf("Switching to QUAD mode\n");
    quad_mode_ = true;
}

void IPS6404StubSm::switch_to_spi()
{
    printf("Switching to SPI mode\n");
    quad_mode_ = false;
}

void IPS6404StubSm::enable_reset()
{
    reset_enabled_ = true;
}

void IPS6404StubSm::reset()
{
    printf("IPS6404StubSm: reset\n");
    reset_enabled_ = false;
}

void IPS6404StubSm::burst_mode_toggle(const evTransmit &ev)
{
    printf("TODO: Implement burst mode toggle\n");
}

void IPS6404StubSm::read_eid(const evTransmit &ev)
{
    printf("Reading EID\n");

    std::size_t i = command_offset + address_offset;
    if (ev.dest.size() < i + 2)
    {
        std::abort();
    }
    ev.dest[i++] = 0x0d;
    ev.dest[i++] = 0x5d;
}

bool IPS6404StubSm::is_quad_mode() const
{
    return quad_mode_;
}

bool IPS6404StubSm::is_reset_enabled() const
{
    return reset_enabled_;
}

IPS6404Stub::IPS6404Stub(std::string_view name)
    : memory_{}
    , sm_data_{memory_}
    , sm_{sm_data_}
{
    printf("Allocating shared memory for: %s\n", name.data());

    memory_.fd = shm_open(name.data(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (memory_.fd < 0)
    {
        panic("Can't open shared memory object: %s\n", strerror(errno));
    }
    memory_.name               = name;
    std::string semaphore_name = "/";
    semaphore_name += name;
    memory_.sem = sem_open(semaphore_name.c_str(), O_CREAT, 0644, 0);
    if (memory_.sem == nullptr)
    {
        panic("Semaphore aquisition failed: %s\n", strerror(errno));
    }

    ftruncate(memory_.fd, memory_size);
    void *mem = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, memory_.fd, 0);

    if (mem == MAP_FAILED)
    {
        panic("Memory mapping failed: %s\n", strerror(errno));
    }

    memory_.memory = std::span<uint8_t>(static_cast<uint8_t *>(mem), memory_size);

    sm_.process_event(IPS6404StubSm::evInit{});
}

IPS6404Stub::~IPS6404Stub()
{
    sem_close(memory_.sem);
    munmap(memory_.memory.data(), memory_size);
    shm_unlink(memory_.name.data());
}

void IPS6404Stub::init()
{
}

void IPS6404Stub::read(const DataType &buf, std::size_t len)
{
    sm_.process_event(IPS6404StubSm::evTransmit{
        .src = ConstDataType{}, .dest = buf, .src_len = 0, .dest_len = len});
}

void IPS6404Stub::write(const ConstDataType &buf, std::size_t len)
{
    sm_.process_event(
        IPS6404StubSm::evTransmit{.src = buf, .dest = DataType{}, .src_len = len, .dest_len = 0});
}

void IPS6404Stub::transmit(const ConstDataType &src, const DataType &dest, std::size_t write_len,
                           std::size_t read_len)
{
    sm_.process_event(IPS6404StubSm::evTransmit{
        .src = src, .dest = dest, .src_len = write_len, .dest_len = read_len});
    // ::write(out_memory_fd_, src.data(), write_len);
    // ::read(in_memory_fd_, dest.data(), read_len);
}

} // namespace stubs
} // namespace msgpu
