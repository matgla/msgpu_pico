// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it is under the terms of the GNU General Public License as published by
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

#pragma once

#include <array>
#include <bitset>
#include <cstdint>

#include "buffers/id_generator.hpp"
#include "memory/gpuram.hpp"

#include "log/log.hpp"

namespace msgpu::buffers
{

struct BufferEntry
{
    uint16_t address;
    uint16_t blocks;
};

class GpuBuffersBase : public IdGenerator<2048>
{
  protected:
    constexpr static std::size_t block_size  = 1024;
    constexpr static std::size_t buffer_size = 2048;

  public:
    void allocate_names(uint32_t amount, uint16_t *ids);

    void release_names(uint32_t amount, uint16_t *ids);

    void allocate_memory(uint32_t id, std::size_t size);
    void deallocate_memory(uint32_t id);

  protected:
    uint32_t find_empty_block(uint32_t size);

    void alloc(BufferEntry &entry, std::size_t size);
    void dealloc(BufferEntry &entry);

    std::bitset<buffer_size> allocation_map_;
    std::array<BufferEntry, buffer_size> entries_;
};

template <typename MemoryType>
class GpuBuffers : public GpuBuffersBase
{
  private:
    constexpr static std::size_t block_size  = 1024;
    constexpr static std::size_t buffer_size = 2048;

  public:
    GpuBuffers(MemoryType &memory)
        : memory_(memory)
    {
    }

    void write(uint32_t id, const void *data, std::size_t size, std::size_t offset = 0)
    {
        printf("Write id: %d\n", id);
        if (!names_map_.test(id))
        {
            return;
        }

        auto &entry = entries_[id];
        printf("write: 0x%lx\n", entry.address + offset);

        memory_.write(entry.address + offset, data, size);
    }

    void read(uint32_t id, void *data, std::size_t size, std::size_t offset = 0)
    {
        if (!names_map_.test(id))
        {
            return;
        }

        auto &entry = entries_[id];

        memory_.read(entry.address + offset, data, size);
    }

  private:
    MemoryType &memory_;
};

} // namespace msgpu::buffers
