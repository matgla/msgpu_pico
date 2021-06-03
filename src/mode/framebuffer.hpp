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

#include "mode/buffer.hpp"

namespace msgpu::mode 
{

template <typename Configuration, bool uses_color_palette>
struct BufferTypeGeneratorImpl;

template <typename Configuration>
struct BufferTypeGenerator
{
    using type = typename BufferTypeGeneratorImpl<Configuration, Configuration::uses_color_palette>::type;
};

template <typename Configuration> 
struct BufferTypeGeneratorImpl<Configuration, true> 
{
    using type = Buffer<Configuration::resolution_width, Configuration::resolution_height, Configuration::bits_per_pixel>;
};

template <typename Configuration>
struct BufferTypeGeneratorImpl<Configuration, false>
{
    using type = std::array<std::array<typename Configuration::ColorType, Configuration::resolution_width>, Configuration::resolution_width>;
};


template <typename Configuration, std::size_t N>
class FrameBuffer 
{
public:
    FrameBuffer()
        : write_index_(0)
        , read_index_(0)
    {
    }

    using BufferType = typename BufferTypeGenerator<Configuration>::type;

    BufferType& get_writable_frame() 
    {
        return framebuffer_[write_index_];
    }

    const BufferType& get_readable_frame() const 
    {
        return framebuffer_[read_index_];
    }

    void switch_to_next_write_buffer()
    {
        if (write_index_ + 1 < N)
        {
            ++write_index_;
        } 
        else 
        {
            write_index_ = 0;
        }
    }

    void switch_to_next_read_buffer()
    {
        if (read_index_ + 1 < N)
        {
            ++read_index_;
        } 
        else 
        {
            read_index_ = 0;
        }
    }

private:
    std::size_t write_index_;
    std::size_t read_index_;
    BufferType framebuffer_[2]; 
};

} // namespace msgpu::mode

