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

#include "mode/framebuffer.hpp"

#include <cstring>

#include "messages/clear_screen.hpp"
#include "messages/swap_buffer.hpp"

#include "memory/vram.hpp"


namespace msgpu::mode 
{

template <typename Configuration>
class ModeBase 
{
public:
    virtual ~ModeBase() = default; 

    ModeBase(memory::VideoRam& framebuffer)
        : buffer_id_(0)
        , framebuffer_(framebuffer)
        
    {
        clear_screen(); 
    }

    void process(const ClearScreen&)
    {
        printf("ClearScreen\n");
        clear_screen();
    }

    void process(const SwapBuffer&)
    {
        buffer_id_ = buffer_id_ ? 0 : 1;
        framebuffer_.select_buffer(buffer_id_);
    }

    void render();

protected: 
    void clear_screen()
    {
        std::span<uint16_t> buf(line_buffer_.u16, Configuration::resolution_width);
    
        std::memset(line_buffer_.u8, 0, buf.size() * 2);
        for (uint16_t line = 0; line < Configuration::resolution_height; ++line)
        {
            framebuffer_.write_line(line, buf);
        }
    }

    uint8_t buffer_id_;
    memory::VideoRam& framebuffer_;
};

} // namespace msgpu::mode

