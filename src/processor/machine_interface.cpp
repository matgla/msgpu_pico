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
        case State::init: 
        {

            msgpu::set_usart_dma_buffer(&receive_.payload[0], false);
            msgpu::set_usart_dma_transfer_count(68, true);
            state_ = State::receive_header;
        } break;
        case State::receive_header:
        {
            state_ = State::init;
            printf("Got: ");
            for (const auto b : receive_.payload)
            {
                printf("%x, ", b);
            }
            printf("\n");
            buffer_.push_back(receive_);
            if (buffer_.size() != buffer_.max_size())
            {
                dma_run();
            }
        } break;
        case State::receive_payload:
        {
            //printf("Receive payload: %d\n", receive_.header.id);
            buffer_.push_back(receive_);
            state_ = State::init;
            dma_run();
        } break;
    }
}

void MachineInterface::process_data() 
{
    if (!buffer_.empty())
    {
        printf("Process message\n");
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
    auto& msg = buffer_.front();
    HandlerType handler = handlers_[msg.payload[1]];
    if (handler != nullptr)
    {
        (this->*handler)();
    }
    else 
    {
        printf("Unsupported message id: %d\n", msg.payload[0]);
    }
    buffer_.pop_front();
} 

uint8_t* MachineInterface::get_payload(Message& msg)
{
    return msg.payload + 4;
}

void MachineInterface::change_mode()
{
    auto& change_mode = cast_to<ChangeMode>(get_payload(buffer_.front()));
    mode_->switch_to(static_cast<vga::modes::Modes>(change_mode.mode));
}

void MachineInterface::set_pixel()
{
    auto& set_pixel = cast_to<SetPixel>(get_payload(buffer_.front()));
    mode_->set_pixel(set_pixel.x, set_pixel.y, set_pixel.color);
}

void MachineInterface::draw_line() 
{
    auto& line = cast_to<DrawLine>(get_payload(buffer_.front()));
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
    auto& primitive = cast_to<BeginPrimitives>(get_payload(buffer_.front()));
    mode_->begin_primitives(static_cast<PrimitiveType>(primitive.type));
}

void MachineInterface::end_primitives()
{
   // printf("End primitive\n");
    mode_->end_primitives();
}

void MachineInterface::write_vertex()
{
    auto& vertex = cast_to<WriteVertex>(get_payload(buffer_.front()));
    //printf("write: %f %f %f\n", vertex.x, vertex.y, vertex.z);
    mode_->write_vertex(vertex.x, vertex.y, vertex.z);
}

void MachineInterface::set_perspective()
{
    auto& perspective = cast_to<SetPerspective>(get_payload(buffer_.front()));
    mode_->set_perspective(perspective.view_angle, perspective.aspect, perspective.z_far, perspective.z_near);
}

} // namespace processor 
