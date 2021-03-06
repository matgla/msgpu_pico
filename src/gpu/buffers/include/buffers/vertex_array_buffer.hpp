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

#pragma once

#include <cstdint>

#include "buffers/id_generator.hpp"

namespace msgpu::buffers
{

struct VertexArrayBufferData
{
    uint8_t size       : 7;
    uint8_t normalized : 1;
    uint8_t type;
    uint16_t index;
    uint16_t stride;
};

template <typename MemoryType, std::size_t buffer_size>
class VertexArrayBuffer : public IdGenerator<buffer_size>
{
  private:
    constexpr static inline std::size_t structure_size = sizeof(VertexArrayBufferData);

  public:
    constexpr static inline std::size_t start_address = 0x100000;

    VertexArrayBuffer(MemoryType &memory)
        : memory_(memory)
    {
    }

    void allocate_names(uint32_t amount, uint16_t *ids)
    {
        for (std::size_t i = 0; i < amount; ++i)
        {
            const uint32_t slot = this->allocate_name();
            ids[i]              = static_cast<uint16_t>(slot);
        }
    }

    void release_names(uint32_t amount, uint16_t *ids)
    {
        for (uint32_t i = 0; i < amount; ++i)
        {
            this->release_name(ids[i]);
        }
    }

    VertexArrayBufferData get(uint16_t index) const
    {
        VertexArrayBufferData data{};
        if (index >= buffer_size)
        {
            return data;
        }

        if (!this->test(index))
        {
            return data;
        }

        memory_.read(start_address + structure_size * index, &data, structure_size);

        return data;
    }

    bool set(uint16_t index, const VertexArrayBufferData &data)
    {
        if (index >= buffer_size)
        {
            return false;
        }

        if (!this->test(index))
        {
            return false;
        }

        memory_.write(start_address + structure_size * index, &data, structure_size);
        return true;
    }

  protected:
    MemoryType &memory_;
};

} // namespace msgpu::buffers

