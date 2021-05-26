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
#include "messages/write_text.hpp"
#include "messages/messages.hpp"
#include "messages/set_perspective.hpp"
#include "messages/swap_buffer.hpp"
#include "messages/draw_triangle.hpp"
#include "modes.hpp"

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
        .id = T::id
    };

    auto header_span = std::span<uint8_t>(reinterpret_cast<uint8_t*>(&header), sizeof(header));
    
    write_(header_span);
    if (sizeof(T) > 0) 
    {
        // uint8_t msg_crc = calculate_crc8(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&msg), sizeof(T)));
        //// write_(std::span<const uint8_t>(
            //std::begin(reinterpret_cast<const uint8_t*>(&msg_crc)), 
            //sizeof(msg_crc)));
        //write_(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&msg), sizeof(msg)));
    }
    prev = msgpu::get_millis();
}

void MachineInterface::send_info(const void* payload)
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
    : state_(State::prepare_for_header)
    , write_(write_callback)
    , mode_(mode)
    , update_scheduled_(false)
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
    handlers_[DrawTriangle::id] = &MachineInterface::draw_triangle;
}

void MachineInterface::dma_run()
{
    if (!update_scheduled_) return;
    update_scheduled_ = false;
    switch (state_)
    {
        case State::prepare_for_header: 
        {
            msgpu::set_usart_dma_buffer(header_buffer_.data(), false);
            msgpu::set_usart_dma_transfer_count(sizeof(Header), true);
            msgpu::reset_dma_crc();
            std::memset(header_buffer_.data(), 0, header_buffer_.size());
            state_ = State::synchronize_header;
        } break;
        case State::synchronize_header: 
        {
            // If start symbol is not received at start, we need to sync 
            std::size_t difference = 0;

           for (std::size_t i = 0; i < sizeof(Header); ++i)
            {
                if (header_buffer_[i] == 0x7e)
                {
                    difference = i;
                    break;
                }
            }
            
            header_start_index_ = difference;
            state_ = State::parse_header;
            if (difference != 0) 
            {
                msgpu::set_usart_dma_transfer_count(difference, true);
            }
            else 
            {
                update_scheduled_ = true;
                return;
            }

        } break;
        case State::parse_header: 
        {
            if (header_start_index_ != 0)
            {
                // Buffer is misaligned, so CRC calculated by DMA is not valid 
                header_crc_ = calculate_crc16<ccit_polynomial>(
                    std::span<uint8_t>(&header_buffer_[header_start_index_], sizeof(Header)));
            }
            else 
            {
                header_crc_ = msgpu::get_dma_crc(); 
            }
            
            msgpu::set_usart_dma_buffer(&received_crc_, false);
            msgpu::set_usart_dma_transfer_count(sizeof(received_crc_), true);
            state_ = State::verify_crc;

        } break;
        case State::verify_crc:
        {
            printf("Got CRC: 0x%x\n", received_crc_);
            state_ = State::receive_header;
            if (header_crc_ != received_crc_)
            {
                printf("Header CRC mismatch, calculated: 0x%x, received 0x%x\n", header_crc_, received_crc_);
                state_ = State::prepare_for_header;
            }
            update_scheduled_ = true;
            return;
        } break;
        case State::receive_header:
        {
            Message msg; 
            std::memcpy(&msg.header, &header_buffer_[header_start_index_], sizeof(Header));
            msg.received = false; 

            printf("Header dump: ");
            for (auto b : header_buffer_)
            {
                printf("0x%x, ", b);
            }
            printf("\n");
    

            printf("Got header { id: %d, size: %d }\n", msg.header.id, msg.header.size);
            if (msg.header.size > 64) 
            {
                state_ = State::prepare_for_header;
                update_scheduled_ = true;
                return;
            }

            if (msg.header.size == 0) 
            {
                state_ = State::prepare_for_header; 
                msg.received = true;
                messages_.push_back(msg);
                update_scheduled_ = true;
                return;
            }
            msgpu::reset_dma_crc();
            messages_.push_back(msg);
            
            msgpu::set_usart_dma_buffer(messages_.back().payload.data(), false);
            msgpu::set_usart_dma_transfer_count(msg.header.size, true);
 

            state_ = State::receive_payload_crc;
        } break;
        case State::receive_payload_crc:
        {
            printf ("Payload received: ");
            std::span<const uint8_t> data(messages_.back().payload.data(), messages_.back().header.size);
            for (const auto b : data) 
            {
                printf("0x%x,", b);
            }
            printf("\n");
            message_crc_ = msgpu::get_dma_crc();
            msgpu::set_usart_dma_buffer(&received_crc_, false);
            msgpu::set_usart_dma_transfer_count(sizeof(received_crc_), true);
            state_ = State::verify_payload_crc;
        } break;
        case State::verify_payload_crc:
        {
            state_ = State::prepare_for_header;
            if (message_crc_ != received_crc_)
            {
                printf("Message CRC failed, recived: 0x%x, expected 0x%x\n", received_crc_, message_crc_);
                messages_.pop_back();
            }
            else 
            {
                printf("Message received\n");
                messages_.back().received = true;
            }
            update_scheduled_ = true;
            return;
        }
    }
}

void MachineInterface::process_data() 
{
        bool received = !messages_.empty() && messages_.front().received;
        if (received)
        {
            printf("Process message\n");
            
            Message msg = messages_.front();
            std::memcpy(msg.payload.data(), messages_.front().payload.data(), messages_.front().header.size);
            messages_.pop_front();
            process_message(msg);
        }
}
void MachineInterface::process(uint8_t byte)
{
}

template <typename Message>
const Message& cast_to(const void* memory)
{
    return *static_cast<const Message*>(memory);
}

void MachineInterface::process_message(const Message& msg)
{
    HandlerType handler = handlers_[msg.header.id];
    if (handler != nullptr)
    {
        (this->*handler)(msg.payload.data());
    }
    else 
    {
        printf("Unsupported message id: %d\n", msg.header.id);
    }
} 

void MachineInterface::change_mode(const void* payload)
{
    auto& change_mode = cast_to<ChangeMode>(payload);
    mode_->switch_to(static_cast<vga::modes::Modes>(change_mode.mode));
}

void MachineInterface::set_pixel(const void* payload)
{
    auto& set_pixel = cast_to<SetPixel>(payload);
    mode_->set_pixel(set_pixel.x, set_pixel.y, set_pixel.color);
}

void MachineInterface::draw_line(const void* payload) 
{
    auto& line = cast_to<DrawLine>(payload);
    mode_->draw_line(line.x1, line.y1, line.x2, line.y2);
}

void MachineInterface::swap_buffers(const void* payload)
{
    //printf("Swap buffer\n");
    mode_->swap_buffer();
}

void MachineInterface::clear_screen(const void* payload)
{
    mode_->clear();
}

void MachineInterface::begin_primitives(const void* payload)
{
    //printf("BeginPrimitives\n");
    auto& primitive = cast_to<BeginPrimitives>(payload);
    mode_->begin_primitives(static_cast<PrimitiveType>(primitive.type));
}

void MachineInterface::end_primitives(const void* payload)
{
   // printf("End primitive\n");
    mode_->end_primitives();
}

void MachineInterface::write_vertex(const void* payload)
{
    auto& vertex = cast_to<WriteVertex>(payload);
    //printf("write: %f %f %f\n", vertex.x, vertex.y, vertex.z);
    mode_->write_vertex(vertex.x, vertex.y, vertex.z);
}

void MachineInterface::set_perspective(const void* payload)
{
    auto& perspective = cast_to<SetPerspective>(payload);
    mode_->set_perspective(perspective.view_angle, perspective.aspect, perspective.z_far, perspective.z_near);
}

void MachineInterface::draw_triangle(const void* payload)
{
    auto& draw = cast_to<DrawTriangle>(payload);
    bool fill = draw.fill == FillType::Solid ? true : false;
    mode_->draw_triangle(draw.x1, draw.y1, draw.x2, draw.y2, draw.x3, draw.y3, fill, draw.color);
}

void MachineInterface::schedule_update() 
{
    update_scheduled_ = true;    
}

} // namespace processor 
