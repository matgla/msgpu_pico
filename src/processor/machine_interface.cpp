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

#include <unistd.h>

#include "messages/header.hpp"
#include "messages/info_resp.hpp"
#include "messages/change_mode.hpp"

#include "messages/messages.hpp"

#include "modes/mode_types.hpp"

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
        printf("%d\n", id);
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
        printf("%d\n", id);
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
    
    auto header_span = std::span<uint8_t>(reinterpret_cast<uint8_t*>(&header), sizeof(header));
    
    write_(header_span);
    write_(std::span<uint8_t>(reinterpret_cast<uint8_t*>(&resp), sizeof(resp)));
}

MachineInterface::MachineInterface(vga::Mode* mode, WriteCallback write_callback)
    : state_(State::receive_header)
    , buffer_counter_(0)
    , size_to_get_(sizeof(Header))
    , write_(write_callback)
    , mode_(mode)
{
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
                printf("Got message header {id: %d, size: %d}\n", header_.id, header_.size);
                buffer_counter_ = 0;
                if (header_.size != 0)
                {
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
                process_message();
                state_ = State::receive_header;
            }
        } break;
    }
}

void MachineInterface::process_message()
{
    Messages msg = static_cast<Messages>(header_.id);
    printf("Process message: %d\n", header_.id);
    switch (msg)
    {
        case Messages::InfoReq: 
        {
            send_info();
        } break;
        case Messages::ChangeMode:
        {
            auto* change_mode = reinterpret_cast<ChangeMode*>(buffer_);
            printf("Change mode to: %d\n", change_mode->mode);
            mode_->switch_to(static_cast<vga::modes::Modes>(change_mode->mode));
        } break;
        default: 
        {
        }
    }
}

} // namespace processor 
