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

#include "sync.hpp"

namespace processor 
{

struct Message 
{
    bool received;
    Header header; 
    std::array<uint8_t, 64> payload;
};

class MachineInterface
{
public:
    using WriteCallback = void(*)(std::span<const uint8_t>);
    MachineInterface(vga::Mode* mode, WriteCallback write_callback);

    void process(uint8_t byte);
    void process_data();
    void dma_run();
    void schedule_update();

private:
    
    template <typename T>
    void send_message(const T& msg);

    void process_message(const Message& msg); 

    typedef void(MachineInterface::*HandlerType)(const void* data);

    void send_info(const void* payload);
    void change_mode(const void* payload);

    void swap_buffers(const void* payload);
    void clear_screen(const void* payload);

    // 2D API 
    void set_pixel(const void* payload);
    void draw_line(const void* payload);
    void draw_triangle(const void* payload);

    // 3d GPU API 
    
    void begin_primitives(const void* payload);
    void end_primitives(const void* payload);
    void write_vertex(const void* payload);
    void set_perspective(const void* payload);

    uint8_t* get_payload(Message& msg);

    uint8_t* get_next_buffer();

    enum class State : uint8_t 
    {
        prepare_for_header,
        synchronize_header,
        parse_header,
        verify_crc,
        receive_header,
        receive_payload,
        receive_payload_crc,
        verify_payload_crc,
        receive_crc
    };

    State state_;
    std::array<uint8_t, sizeof(Header)*2> header_buffer_;
    uint8_t header_start_index_;
    uint16_t header_crc_;
    uint16_t message_crc_;
    uint16_t received_crc_;
    Header current_header_;
    eul::container::static_deque<Message, 32> messages_;
    WriteCallback write_;
    vga::Mode* mode_;
    bool update_scheduled_; 

    std::array<HandlerType, 255> handlers_;
    mutex_t mutex_;
};

} // namespace processor

