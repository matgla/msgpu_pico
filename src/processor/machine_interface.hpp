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

#include "messages/messages.hpp"

namespace processor 
{

class MachineInterface
{
public:
    MachineInterface();

    void process(uint8_t byte);

private:
    enum class State : uint8_t 
    {
        waiting_for_id,
        receiving_command, 
        processing_command
    };

    State state_;
    char buffer_[255];
    std::size_t size_to_get_;
    Messages message_id_;
};

} // namespace processor

