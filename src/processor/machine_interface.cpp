// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik 
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

#include "processor/machine_interface.hpp"

#include <cstring>
#include <cstdio> 

#include <eul/crc/crc.hpp>

#include <unistd.h>

#include "messages/header.hpp"
#include "messages/info_resp.hpp"
#include "messages/change_mode.hpp"
#include "messages/set_pixel.hpp"
#include "messages/draw_line.hpp" 
#include "messages/info_req.hpp" 
#include "messages/clear_screen.hpp"
#include "messages/begin_primitives.hpp"
#include "messages/end_primitives.hpp"
#include "messages/write_vertex.hpp"
#include "messages/messages.hpp"
#include "messages/set_perspective.hpp"
#include "messages/swap_buffer.hpp"
#include "modes/mode_types.hpp"

#include "board.hpp"

namespace processor 
{

template <typename type, int id> 
void fill_data(InfoResp* info) 
{
    info->modes[id].used = true;
    info->modes[id].id = static_cast<uint8_t>(type::ConfigurationType::mode);
    info->modes[id].resolution_width = type::ConfigurationType::width;
    info->modes[id].resolution_height = type::ConfigurationType::height;

    info->modes[id].color_depth =  1 << type::ConfigurationType::bits_per_pixel;
    info->modes[id].mode = std::is_same_v<typename type::type, vga::modes::Text> ? Mode::Text : Mode::Graphic;
    info->modes[id].uses_color_palette = type::ConfigurationType::uses_color_palette;
}

template <int id, typename type, typename... types> 
struct fill_modes_impl
{
    static void fill(InfoResp* info)
    {
        fill_data<type, id>(info);
        fill_modes_impl<id + 1, types...>::fill(info);
    }
};

template <int id, typename type>
struct fill_modes_impl<id, type>
{
    static void fill(InfoResp* info)
    {
        fill_data<type, id>(info);
    }
};

template <typename tuple>
struct fill_modes 
{
};

template <typename... types>
struct fill_modes<std::tuple<types...>>
{
    static void iterate(InfoResp* info)
    {
        fill_modes_impl<0, types...>::fill(info);
    }
};


void MachineInterface::send_info()
{
    InfoResp resp {
        .version_major = 1, 
        .version_minor = 0
    };

    std::memset(resp.modes, 0, sizeof(resp.modes));
    fill_modes<vga::Mode::ModeTypes>::iterate(&resp);

    resp.modes[0] = {
        .uses_color_palette = true,
        .mode = Mode::Text,
        .used = true,
        .id = 1,
        .resolution_width = 80,
        .resolution_height = 30,
        .color_depth = 16 
    };

    Header header = {
        .id = static_cast<uint8_t>(Messages::InfoResp),
        .size = sizeof(InfoResp)
    };

    header.crc = calculate_crc8(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&header), 3));
    
    auto header_span = std::span<uint8_t>(reinterpret_cast<uint8_t*>(&header), sizeof(header));
    
    write_(header_span);
    write_(std::span<uint8_t>(reinterpret_cast<uint8_t*>(&resp), sizeof(resp)));
}

MachineInterface::MachineInterface(vga::Mode* mode, WriteCallback write_callback)
    : got_data_(false)
    , state_(State::init)
    , buffer_counter_(0)
    , size_to_get_(sizeof(Header))
    , write_(write_callback)
    , mode_(mode)
{
    handlers_.fill(0);
    handlers_[ChangeMode::id] = &MachineInterface::change_mode; 
    handlers_[InfoReq::id] = &MachineInterface::send_info;
    handlers_[SetPixel::id] = &MachineInterface::set_pixel;
    handlers_[DrawLine::id] = &MachineInterface::draw_line;
    handlers_[ClearScreen::id] = &MachineInterface::clear_screen;
    handlers_[BeginPrimitives::id] = &MachineInterface::begin_primitives;
    handlers_[EndPrimitives::id] = &MachineInterface::end_primitives;
    handlers_[WriteVertex::id] = &MachineInterface::write_vertex;
    handlers_[SetPerspective::id] = &MachineInterface::set_perspective;
    handlers_[SwapBuffer::id] = &MachineInterface::swap_buffers;
}

void MachineInterface::dma_run()
{
    switch (state_)
    {
        case State::init: 
        {
       //     printf("Waiting for header\n");
            msgpu::set_usart_dma_buffer(&header_buffer_[0], false);
            msgpu::set_usart_dma_transfer_count(sizeof(Header), true);
            state_ = State::receive_header;
        } break;
        case State::receive_header:
        {
            std::memcpy(&header_, header_buffer_, sizeof(Header));
            uint8_t crc = calculate_crc8(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&header_buffer_[0]), 3));
            //printf("Got header id: %d, size %d\n", header_.id, header_.size);
            if (crc != header_.crc)
            {
                printf("CRC validation failed. Expected 0x%x, got: 0x%x\n", crc, header_.crc);
                state_ = State::init;
                dma_run();
                return;
            }
            else 
            {
                if (header_.size == 0)
                {
                  //  printf("Process message with header 0\n");
                    state_ = State::init;
                    got_data_ = true; 
                    return;
                }
                if (header_.size >= sizeof(buffer_))
                {
                    state_ = State::init;
                    dma_run();
                    return;
                }
                msgpu::set_usart_dma_buffer(buffer_, false);
                msgpu::set_usart_dma_transfer_count(header_.size, true);
                state_ = State::receive_payload;
            }
        } break;
        case State::receive_payload:
        {
            msgpu::set_usart_dma_buffer(reinterpret_cast<uint8_t*>(&message_crc_), false);
            msgpu::set_usart_dma_transfer_count(sizeof(message_crc_), true);
            state_ = State::receive_crc;
        } break;
        case State::receive_crc:
        {
            const uint8_t msg_crc = calculate_crc8(std::span<const uint8_t>(&buffer_[0], header_.size));
            //printf("Got msg crc 0x%x\n", msg_crc);
            

            if (msg_crc != message_crc_)
            {
                printf("CRC mismatch, received 0x%x, expected 0x%x. Dropping message\n", message_crc_, msg_crc);
                printf("Data: ");
                for (std::size_t i = 0; i < header_.size; ++i)
                {
                    printf("%d, ", buffer_[i]);
                }
                printf("\n");
            }
            else 
            {
                //printf("Process message: %d\n", header_.id);
                got_data_ = true;
                state_ = State::init;
                return;
                // just indicate 
                //        process_message();
            }

            state_ = State::init;
            dma_run();
        } break;
    }
}

void MachineInterface::process_data() 
{
    if (got_data_)
    {
        process_message();
        got_data_ = false;
        dma_run();
    }
}
void MachineInterface::process(uint8_t byte)
{
    switch (state_)
    {
        case State::receive_header:
        {
            header_buffer_[buffer_counter_] = byte;  
            if (buffer_counter_ == sizeof(Header) - 1)
            {
                std::memcpy(&header_, header_buffer_, sizeof(Header));
                buffer_counter_ = 0;

                uint8_t crc = calculate_crc8(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&header_buffer_[0]), 3));
                if (crc != header_.crc)
                {
                    printf("CRC validation failed. Expected 0x%x, got: 0x%x\n", crc, header_.crc);
                    return;
                }
 
                if (header_.size != 0)
                {
                    printf("Received header %d, size %d\n", header_.id, header_.size);

                    state_ = State::receive_payload;
                }
                else 
                {
                    process_message();
                }
            }
            else 
            {
                ++buffer_counter_;
            }
        } break;
        case State::receive_payload:
        {
            buffer_[buffer_counter_] = byte;
            ++buffer_counter_;
            if (buffer_counter_ >= header_.size)
            {
                buffer_counter_ = 0;
                state_ = State::receive_crc;
            }
        } break;
        case State::receive_crc: 
        {
            *(reinterpret_cast<uint8_t*>(&message_crc_) + buffer_counter_) = byte;
            if (++buffer_counter_ >= 4)
            {
                const uint32_t msg_crc = calculate_crc32(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&buffer_[0]), header_.size));
                printf("Got msg crc 0x%x\n", msg_crc);
                
                if (msg_crc != message_crc_)
                {
                    printf("CRC mismatch, received 0x%x, expected 0x%x. Dropping message\n", message_crc_, msg_crc);
                }
                else 
                {
                    printf("Process message: %d\n", header_.id);
                    process_message();
                }

                buffer_counter_ = 0;
                state_ = State::receive_header;
            }
 
        } break;
    }
}

template <typename Message>
Message& cast_to(void* memory)
{
    return *static_cast<Message*>(memory);
}

void MachineInterface::process_message()
{
    static uint32_t prev = 0;   
    uint32_t n = msgpu::get_millis(); 
    //printf("Between message: %d ms\n", (n-prev));
    HandlerType handler = handlers_[header_.id];
    if (handler != nullptr)
    {
        (this->*handler)();
    }
    else 
    {
        printf("Unsupported message id: %d\n", header_.id);
    }
    prev = msgpu::get_millis(); 

} 

void MachineInterface::change_mode()
{
    auto& change_mode = cast_to<ChangeMode>(buffer_);
    mode_->switch_to(static_cast<vga::modes::Modes>(change_mode.mode));
}

void MachineInterface::set_pixel()
{
    auto& set_pixel = cast_to<SetPixel>(buffer_);
    mode_->set_pixel(set_pixel.x, set_pixel.y, set_pixel.color);
}

void MachineInterface::draw_line() 
{
    auto& line = cast_to<DrawLine>(buffer_);
    mode_->draw_line(line.x1, line.y1, line.x2, line.y2);
}

void MachineInterface::swap_buffers()
{
    //printf("Swap buffer\n");
    mode_->swap_buffer();
}

void MachineInterface::clear_screen()
{
    mode_->clear();
}

void MachineInterface::begin_primitives()
{
    //printf("BeginPrimitives\n");
    auto& primitive = cast_to<BeginPrimitives>(buffer_);
    mode_->begin_primitives(static_cast<PrimitiveType>(primitive.type));
}

void MachineInterface::end_primitives()
{
   // printf("End primitive\n");
    mode_->end_primitives();
}

void MachineInterface::write_vertex()
{
    auto& vertex = cast_to<WriteVertex>(buffer_);
    //printf("write: %f %f %f\n", vertex.x, vertex.y, vertex.z);
    mode_->write_vertex(vertex.x, vertex.y, vertex.z);
}

void MachineInterface::set_perspective()
{
    auto& perspective = cast_to<SetPerspective>(buffer_);
    mode_->set_perspective(perspective.view_angle, perspective.aspect, perspective.z_far, perspective.z_near);
}

} // namespace processor 
