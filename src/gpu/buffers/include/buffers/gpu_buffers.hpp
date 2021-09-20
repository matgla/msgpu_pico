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

#include "memory/gpuram.hpp"

namespace msgpu::buffers
{

struct BufferEntry
{
    uint16_t address;
    uint16_t blocks;
};

class GpuBuffersBase
{
  private:
    constexpr static std::size_t block_size  = 1024;
    constexpr static std::size_t buffer_size = 2048;

  public:
    void allocate_names(uint32_t amount, uint32_t *ids);

    void release_names(uint32_t amount, uint32_t *ids);

  private:
    uint32_t find_empty_block(uint32_t size);

    void alloc(BufferEntry &entry, std::size_t size);
    void dealloc(BufferEntry &entry, uint32_t block);

    uint32_t find_empty_slot();

    std::bitset<buffer_size> allocation_map_;
    std::bitset<buffer_size> entries_map_;
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

    void write(uint32_t id, const void *data, std::size_t size)
    {
    }

  private:
    MemoryType &memory_;
};

} // namespace msgpu::buffers
