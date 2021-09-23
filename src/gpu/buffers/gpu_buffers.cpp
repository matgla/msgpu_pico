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

#include "buffers/gpu_buffers.hpp"

namespace msgpu::buffers
{

void GpuBuffersBase::allocate_names(uint32_t amount, uint16_t *ids)
{
    for (std::size_t i = 0; i < amount; ++i)
    {
        const uint32_t slot = allocate_name();
        ids[i]              = static_cast<uint16_t>(slot);
        entries_[slot]      = BufferEntry{};
    }
}

void GpuBuffersBase::release_names(uint32_t amount, uint16_t *ids)
{
    for (uint32_t i = 0; i < amount; ++i)
    {
        release_name(ids[i]);
        dealloc(entries_[ids[i]]);
    }
}

void GpuBuffersBase::allocate_memory(uint32_t id, std::size_t size)
{
    if (!names_map_.test(id))
    {
        return;
    }

    auto &entry = entries_[id];
    if (entry.blocks)
    {
        dealloc(entry);
    }

    alloc(entry, size);
}

uint32_t GpuBuffersBase::find_empty_block(uint32_t size)
{
    uint32_t current_size = 0;
    uint32_t block_begin  = 0;

    for (uint32_t i = 0; i < buffer_size; ++i)
    {
        if (allocation_map_[i] == 0)
        {
            ++current_size;
            if (current_size == size)
            {
                return block_begin;
            }
        }
        else
        {
            current_size = 0;
            block_begin  = i + 1;
        }
    }
    return 0xffffffff;
}

void GpuBuffersBase::alloc(BufferEntry &entry, std::size_t size)
{
    if (size == 0)
    {
        return;
    }
    const uint32_t size_in_blocks =
        static_cast<uint32_t>(size / block_size) + (size % block_size != 0);
    const uint32_t start_block = find_empty_block(size_in_blocks);

    for (uint32_t i = start_block; i < start_block + size_in_blocks; ++i)
    {
        allocation_map_[i] = 1;
    }

    entry.blocks  = static_cast<uint16_t>(size_in_blocks);
    entry.address = static_cast<uint16_t>(start_block * block_size);

    log::Log::trace("Allocated memory: { address: 0x%x, blocks: %d, size: %d}", entry.address,
                    entry.blocks, entry.blocks * block_size);
}

void GpuBuffersBase::dealloc(BufferEntry &entry)
{
    const uint32_t start_block = entry.address / block_size;
    for (uint32_t i = start_block; i < start_block + entry.blocks; ++i)
    {
        allocation_map_[i] = 0;
    }

    entry.address = 0;
    entry.blocks  = 0;
}

} // namespace msgpu::buffers

