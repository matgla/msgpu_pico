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

#include <cstdint>
#include <span>

#include "messages/messages.hpp"

#include "modes/modes.hpp"

#include "messages/header.hpp"

namespace processor 
{

class MachineInterface
{
public:
    using WriteCallback = void(*)(std::span<uint8_t>);
    MachineInterface(vga::Mode* mode, WriteCallback write_callback);

    void process(uint8_t byte);

private:

    void send_info();
    void process_message(); 

    enum class State : uint8_t 
    {
        receive_header,
        receive_payload 
    };

    State state_;
    char buffer_[255];
    std::size_t buffer_counter_;
    std::size_t size_to_get_;
    Messages message_id_;
    WriteCallback write_;
    vga::Mode* mode_;

    char header_buffer_[sizeof(Header)];
    Header header_;
};

} // namespace processor

