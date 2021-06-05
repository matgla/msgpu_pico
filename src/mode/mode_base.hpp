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

#include "messages/clear_screen.hpp"
#include "messages/swap_buffer.hpp"

namespace msgpu::mode 
{

template <typename Configuration, std::size_t BufferSize>
class ModeBase : public FrameBuffer<Configuration, BufferSize>
{
public:
    virtual ~ModeBase() = default; 

    ModeBase()
    {
        if (BufferSize > 1)
        {
            buffer_.switch_to_next_write_buffer();
        }
    }

    void process(const ClearScreen&)
    {
        printf("ClearScreen\n");
        auto& frame = buffer_.get_writable_frame();
        for (auto& line : frame) 
        {
            line.fill(0xf);
        }
    }

    void process(const SwapBuffer&)
    {
        if (BufferSize == 1) return;
        buffer_.switch_to_next_write_buffer();
        buffer_.switch_to_next_read_buffer();
    }

    std::span<const uint8_t> get_line(std::size_t line_number) const 
    {
        const auto& frame = buffer_.get_readable_frame();
        const auto& line = frame[line_number];
        return std::span<const uint8_t>(line.begin(), line.end());
    }

    void render();

protected: 

    FrameBuffer<Configuration, BufferSize> buffer_;
};

} // namespace msgpu::mode

