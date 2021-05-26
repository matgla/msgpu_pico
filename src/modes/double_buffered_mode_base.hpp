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

#include "generator/vga.hpp"

#include "board.hpp"
#include "sync.hpp"

#include "modes/palette_mode_base.hpp"
#include "modes/mode_base.hpp"

namespace vga::modes 
{

template <typename Configuration> 
class DoubleBufferedModeBase 
{
public:
    DoubleBufferedModeBase()
        : read_buffer_id_(1)
        , write_buffer_id_(0)
        , swap_buffers_(false)
    {
        mutex_init(&mutex_);
        get_vga().change_mode(Configuration::mode);
    }

    void clear()
    {
        // protects write_buffer_id_ 
        mutex_enter_blocking(&mutex_);
        for (auto& line : get_writable_frame())
        {
            line.fill(0);
        }
        mutex_exit(&mutex_);
    }
   
    void base_render()
    {
        mutex_enter_blocking(&mutex_);
        if (swap_buffers_)
        {
            std::size_t c = read_buffer_id_;
            read_buffer_id_ = write_buffer_id_;
            write_buffer_id_ = c;
            swap_buffers_ = false; 
        }
        mutex_exit(&mutex_);
    }

    void set_pixel(const Position position, const typename Configuration::Color color)
    {
        if (position.x < 0 || position.x >= Configuration::resolution_width)
        {
            return;
        }

        if (position.y < 0 || position.y >= Configuration::resolution_height)
        {
            return;
        }

        this->framebuffer_[write_buffer_id_][position.y][position.x] = color;
    }

    void swap_buffers()
    {
        mutex_enter_blocking(&mutex_);
        swap_buffers_ = true;
        mutex_exit(&mutex_); 
    }

    using BufferType = typename BufferTypeGenerator<Configuration>::type;

protected:
    BufferType& get_writable_frame() 
    {
        return framebuffer_[write_buffer_id_];
    }

    const BufferType& get_readable_frame() const 
    {
        return framebuffer_[read_buffer_id_];
    }

    uint8_t write_buffer_id_ : 2;
    uint8_t read_buffer_id_ : 2;
    uint8_t swap_buffers_ : 1;
    mutex_t mutex_;
    BufferType framebuffer_[2];
};


template <typename Configuration>
using DoubleBufferedPaletteBase = PaletteModeBase<Configuration, DoubleBufferedModeBase>;

template <typename Configuration>
using DoubleBufferedRawBase = RawModeBase<Configuration, DoubleBufferedModeBase>;


} // namespace vga::modes

