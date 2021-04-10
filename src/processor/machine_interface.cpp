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

#include "processor/machine_interface.hpp"

#include <cstring>

#include <unistd.h>

#include "processor/messages/info_resp.h"
#include "processor/messages/messages.h"

namespace processor 
{

namespace  
{

void send_info()
{
    info_resp resp {
        .version_major = 1, 
        .version_minor = 0
    };

    std::memset(resp.modes, 0, sizeof(resp.modes));

    resp.modes[0] = {
        .uses_color_palette = true,
        .mode = Mode::Text,
        .id = 1,
        .resolution_width = 80,
        .resolution_height = 30,
        .color_depth = 16 
    };

    Message id = Message::info_resp_id;
    write(STDOUT_FILENO, &id, sizeof(id));
    write(STDOUT_FILENO, &resp, sizeof(info_resp));
    
}

} // namespace 

MachineInterface::MachineInterface()
    : state_(State::waiting_for_id)
    , size_to_get_(0)
{
}

void MachineInterface::process(uint8_t byte)
{
    switch (state_)
    {
        case State::waiting_for_id:
        {
            message_id_ = byte; 
            if (message_id_ == Message::info_req_id)
            {
                size_to_get_ = 0;
                send_info();
            }
        } break;
    }
}

} // namespace processor 
