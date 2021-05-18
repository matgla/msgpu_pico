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

#include "messages/ack.hpp"
#include "messages/nack.hpp"
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

static uint32_t prev = 0;

template <typename T>
void MachineInterface::send_message(const T& msg)
{
  //  printf("%d\n", msgpu::get_millis() - prev);
    Header header = {
        .id = T::id,
        .size = sizeof(T)
    };

    header.crc = calculate_crc8(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&header), 3));
    
    auto header_span = std::span<uint8_t>(reinterpret_cast<uint8_t*>(&header), sizeof(header));
    
    for (uint32_t b : header_span)
    {
        printf("%d, ", b);
    }
    printf("\n");
    write_(header_span);
    if (header.size >= 1) 
    {
        uint8_t msg_crc = calculate_crc8(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&msg), sizeof(T)));
        write_(std::span<const uint8_t>(
            reinterpret_cast<const uint8_t*>(&msg_crc), 
            sizeof(msg_crc)));
        write_(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&msg), sizeof(msg)));
    }
    prev = msgpu::get_millis();
}

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

    send_message(resp);
}

MachineInterface::MachineInterface(vga::Mode* mode, WriteCallback write_callback)
    : state_(State::init)
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
            msgpu::set_usart_dma_buffer(reinterpret_cast<uint8_t*>(&receive_.header), false);
            msgpu::set_usart_dma_transfer_count(sizeof(Header), true);
            state_ = State::receive_header;
        } break;
        case State::receive_header:
        {
            uint8_t crc = calculate_crc8(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&receive_.header), 3));
            //printf("Got header id: %d, size %d\n", header_.id, header_.size);
            //
            printf("h %d\n", msgpu::get_millis() - prev);
            prev = msgpu::get_millis();
            if (crc != receive_.header.crc)
            {
                state_ = State::init;
                dma_run();
                return;
            }
            else 
            {
                if (receive_.header.size == 0)
                {
                  //  printf("Process message with header 0\n");
                    state_ = State::init;
                    buffer_.push_back(receive_);
                    send_message(Ack{});
                    if (buffer_.size() != buffer_.max_size())
                    {
                        dma_run();
                    }
                    return;
                }
                if (receive_.header.size >= 255)
                {
                    state_ = State::init;
                    dma_run();
                    return;
                }
                msgpu::set_usart_dma_buffer(receive_.payload.data(), false);
                msgpu::set_usart_dma_transfer_count(receive_.header.size, true);
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
            const uint8_t msg_crc = calculate_crc8(std::span<const uint8_t>(receive_.payload.data(), receive_.header.size));
            //printf("Got msg crc 0x%x\n", msg_crc);
            

            if (msg_crc != message_crc_)
            {
                send_message(Nack{});
            }
            else 
            {
                printf("m %d\n", msgpu::get_millis() - prev);
                send_message(Ack{});
                buffer_.push_back(receive_);
            }

            state_ = State::init;
            dma_run();
        } break;
    }
}

void MachineInterface::process_data() 
{
    if (!buffer_.empty())
    {
        bool rerun = false; 
        if (buffer_.size() == buffer_.max_size())
        {
            rerun = true;
        }

        process_message();
        if (rerun) dma_run();
    }
}
void MachineInterface::process(uint8_t byte)
{
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
    
    auto& msg = buffer_.front();
    HandlerType handler = handlers_[msg.header.id];
    if (handler != nullptr)
    {
        (this->*handler)();
    }
    else 
    {
        printf("Unsupported message id: %d\n", msg.header.id);
    }
    buffer_.pop_front();
    prev = msgpu::get_millis(); 

} 

void MachineInterface::change_mode()
{
    auto& change_mode = cast_to<ChangeMode>(buffer_.front().payload.data());
    mode_->switch_to(static_cast<vga::modes::Modes>(change_mode.mode));
}

void MachineInterface::set_pixel()
{
    auto& set_pixel = cast_to<SetPixel>(buffer_.front().payload.data());
    mode_->set_pixel(set_pixel.x, set_pixel.y, set_pixel.color);
}

void MachineInterface::draw_line() 
{
    auto& line = cast_to<DrawLine>(buffer_.front().payload.data());
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
    auto& primitive = cast_to<BeginPrimitives>(buffer_.front().payload.data());
    mode_->begin_primitives(static_cast<PrimitiveType>(primitive.type));
}

void MachineInterface::end_primitives()
{
   // printf("End primitive\n");
    mode_->end_primitives();
}

void MachineInterface::write_vertex()
{
    auto& vertex = cast_to<WriteVertex>(buffer_.front().payload.data());
    //printf("write: %f %f %f\n", vertex.x, vertex.y, vertex.z);
    mode_->write_vertex(vertex.x, vertex.y, vertex.z);
}

void MachineInterface::set_perspective()
{
    auto& perspective = cast_to<SetPerspective>(buffer_.front().payload.data());
    mode_->set_perspective(perspective.view_angle, perspective.aspect, perspective.z_far, perspective.z_near);
}

} // namespace processor 
