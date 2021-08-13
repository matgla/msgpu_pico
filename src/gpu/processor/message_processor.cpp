// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
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

#include "processor/message_processor.hpp"

#include <cstdio>
namespace msgpu 
{
namespace processor 
{

MessageProcessor::MessageProcessor() 
    : handlers_{}
{
}

void MessageProcessor::process_message(const io::Message& message)
{
    HandlerType& handler = handlers_[message.header.id];
    if (handler)
    {
        printf("Handle message id: %d\n", message.header.id);
        handler(message.payload.data());
    }
    else 
    {
        printf("Unhandled message id: %d\n", message.header.id);
    }
}

} // namespace processor
} // namespace msgpu

