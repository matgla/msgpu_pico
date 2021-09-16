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

#include "messages/ack.hpp"
#include "messages/clear_screen.hpp"
#include "messages/swap_buffer.hpp"

#include "memory/vram.hpp"

#include "io/usart_point.hpp"
#include "generator/vga.hpp"
#include "arch/i2c.hpp"

namespace msgpu::mode 
{

template <typename Configuration>
class ModeBase 
{
public:
    virtual ~ModeBase() = default; 

    ModeBase(memory::VideoRam& framebuffer, I2C& i2c, io::UsartPoint& point)
        : buffer_id_(1)
        , framebuffer_(framebuffer)
        , i2c_(i2c)
        , point_(point)
        
    {
        clear_screen(); 
    }

    virtual void clear() = 0;
    void process(const ClearScreen&)
    {
        // printf("ClearScreen\n");
        clear_screen();
        clear();
    }

    void process(const SwapBuffer&)
    {
        uint8_t read_buf_id = buffer_id_;
        buffer_id_ = buffer_id_ ? 0 : 1;
        render();
       
        const uint8_t cmd[] = {0x03, read_buf_id};
        this->i2c_.write(0x2e, cmd);

        uint8_t ack[2] = {};
        this->i2c_.read(ack); 

        this->point_.write(Ack{});

        framebuffer_.select_buffer(buffer_id_, read_buf_id);
    }

    virtual void render() = 0;

protected: 
    void clear_screen()
    {
    }

    union LineBuffer 
    {
        uint8_t u8[1024];
        uint16_t u16[1024/2];
    };

    uint8_t buffer_id_;
    memory::VideoRam& framebuffer_;
    LineBuffer line_buffer_;
    I2C& i2c_;
    io::UsartPoint& point_;
};

} // namespace msgpu::mode

