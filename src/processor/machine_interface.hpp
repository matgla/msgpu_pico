// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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

#include <array>
#include <cstdint>
#include <span>

#include <eul/functional/function.hpp>

#include "messages/messages.hpp"

#include "modes/modes.hpp"

#include "messages/header.hpp"

#include <eul/container/static_deque.hpp>

namespace processor 
{

struct Message 
{
    uint8_t payload[8];
};

class MachineInterface
{
public:
    using WriteCallback = void(*)(std::span<const uint8_t>);
    MachineInterface(vga::Mode* mode, WriteCallback write_callback);

    void process(uint8_t byte);
    void process_data();
    void dma_run();
private:
    
    template <typename T>
    void send_message(const T& msg);

    void process_message(); 

    typedef void(MachineInterface::*HandlerType)();

    void send_info();
    void change_mode();

    void set_pixel();
    void draw_line();

    void swap_buffers();
    void clear_screen();

    // 3d GPU API 
    
    void begin_primitives();
    void end_primitives();
    void write_vertex();
    void set_perspective();

    uint8_t* get_payload(Message& msg);

    enum class State : uint8_t 
    {
        init,
        receive_header,
        receive_payload,
        receive_crc
    };

    State state_;
   // static uint32_t data_[50];
    eul::container::static_deque<Message, 20> buffer_;
    Message receive_;
    uint8_t message_crc_;
    WriteCallback write_;
    vga::Mode* mode_;
    
    std::array<HandlerType, 255> handlers_;
};

} // namespace processor

