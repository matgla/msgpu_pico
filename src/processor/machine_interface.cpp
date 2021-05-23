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
        .id = T::id
    };

    auto header_span = std::span<uint8_t>(reinterpret_cast<uint8_t*>(&header), sizeof(header));
    
    write_(header_span);
    if (sizeof(T) > 0) 
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
    : state_(State::prepare_for_header)
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

std::size_t get_message_size(const uint8_t id)
{
    Messages msg_id = static_cast<Messages>(id);

    switch (msg_id) 
    {
        case Messages::Ack: return sizeof(Ack);
        case Messages::BeginPrimitives: return sizeof(BeginPrimitives);
        case Messages::ChangeMode: return sizeof(ChangeMode);
        case Messages::ClearScreen: return sizeof(ClearScreen);
        case Messages::DrawLine: return sizeof(DrawLine);
        case Messages::EndPrimitives: return sizeof(EndPrimitives);
        case Messages::InfoReq: return sizeof(InfoReq);
        case Messages::InfoResp: return sizeof(InfoResp);
        case Messages::Nack: return sizeof(Nack);
        case Messages::SetPerspective: return sizeof(SetPerspective);
        case Messages::SetPixel: return sizeof(SetPixel);
        case Messages::SwapBuffer: return sizeof(SwapBuffer);
        case Messages::WriteText: return sizeof(WriteText);
        case Messages::WriteVertex: return sizeof(WriteVertex);
    }
    return 0;
};

void MachineInterface::dma_run()
{
    switch (state_)
    {
        case State::prepare_for_header: 
        {
            printf("Prepare for header\n");
            msgpu::set_usart_dma_buffer(header_buffer_.data(), false);
            msgpu::set_usart_dma_transfer_count(sizeof(Header), true);
            msgpu::reset_dma_crc();
            state_ = State::synchronize_header;
        } break;
        case State::synchronize_header: 
        {
            // If start symbol is not received at start, we need to sync 
            std::size_t difference = 0;
            printf("Potential header get: ");
            for (auto b : header_buffer_) 
            {
                printf("%d,", b);
            }
            printf("\n");
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
                printf("Synchronize i: %d\n", difference);
                msgpu::set_usart_dma_transfer_count(difference, true);
            }
            else 
            {
                dma_run();
            }

        } break;
        case State::parse_header: 
        {
            printf("Parse header: %d\n", header_start_index_);
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
            printf("Going to verify\n");
            state_ = State::verify_crc;

        } break;
        case State::verify_crc:
        {
            printf("Got CRC: 0x%x\n", received_crc_);
            state_ = State::receive_header;
            if (header_crc_ != received_crc_)
            {
                printf("Header CRC mismatch, calculated: 0x%x, received 0x%x\n", header_crc_, received_crc_);
            }
            dma_run();
        } break;
        case State::receive_header:
        {
            Message msg; 
            std::memcpy(&msg.header, &header_buffer_[header_start_index_], sizeof(Header));
            msg.received = false; 

            printf("Got header { id: %d, size: %d }\n", msg.header.id, msg.header.size);

            if (msg.header.size > 64) 
            {
                state_ = State::prepare_for_header;
                return;
            }
            messages_.push_back(msg);

            msgpu::set_usart_dma_buffer(messages_.back().payload.data(), false);
            msgpu::set_usart_dma_transfer_count(msg.header.size, true);
            msgpu::reset_dma_crc();
            state_ = State::receive_payload_crc;
        } break;
        case State::receive_payload_crc:
        {
            printf("Receive payload\n");
            message_crc_ = msgpu::get_dma_crc();
            msgpu::set_usart_dma_buffer(&received_crc_, false);
            msgpu::set_usart_dma_transfer_count(sizeof(received_crc_), true);
            state_ = State::verify_payload_crc;
        } break;
        case State::verify_payload_crc:
        {
            printf("Verify payload\n");
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
            dma_run();
        }
    }
}

void MachineInterface::process_data() 
{
    if (!messages_.empty())
    {
        if (messages_.front().received)
        {
            printf("Process message\n");
            process_message();
        }
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
    const auto& msg = messages_.front();
    HandlerType handler = handlers_[msg.header.id];
    if (handler != nullptr)
    {
        (this->*handler)();
    }
    else 
    {
        printf("Unsupported message id: %d\n", msg.header.id);
    }
    messages_.pop_front();
} 

void MachineInterface::change_mode()
{
    auto& change_mode = cast_to<ChangeMode>(messages_.front().payload.data());
    mode_->switch_to(static_cast<vga::modes::Modes>(change_mode.mode));
}

void MachineInterface::set_pixel()
{
    auto& set_pixel = cast_to<SetPixel>(messages_.front().payload.data());
    mode_->set_pixel(set_pixel.x, set_pixel.y, set_pixel.color);
}

void MachineInterface::draw_line() 
{
    auto& line = cast_to<DrawLine>(messages_.front().payload.data());
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
    auto& primitive = cast_to<BeginPrimitives>(messages_.front().payload.data());
    mode_->begin_primitives(static_cast<PrimitiveType>(primitive.type));
}

void MachineInterface::end_primitives()
{
   // printf("End primitive\n");
    mode_->end_primitives();
}

void MachineInterface::write_vertex()
{
    auto& vertex = cast_to<WriteVertex>(messages_.front().payload.data());
    //printf("write: %f %f %f\n", vertex.x, vertex.y, vertex.z);
    mode_->write_vertex(vertex.x, vertex.y, vertex.z);
}

void MachineInterface::set_perspective()
{
    auto& perspective = cast_to<SetPerspective>(messages_.front().payload.data());
    mode_->set_perspective(perspective.view_angle, perspective.aspect, perspective.z_far, perspective.z_near);
}

} // namespace processor 
