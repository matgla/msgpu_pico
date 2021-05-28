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

#pragma once 

#include <eul/functional/function.hpp>

#include "io/message.hpp"

namespace msgpu 
{
namespace processor 
{


template <typename BindedType, typename MsgType>
struct HandlerBinder 
{
    typedef void (BindedType::*fun)(const MsgType& payload);
    
    HandlerBinder(fun f, BindedType* b) : f_(f), self_(b)
    {
    }

    void operator()(const void* payload) const
    {
        (self_->*f_)(*static_cast<const MsgType*>(payload));
    }

private:
    fun f_;
    BindedType* self_;
};

class MessageProcessor 
{
public: 
    MessageProcessor();

    void process_message(const io::Message& message);
 
    template <typename MessageType>
    void register_handler(auto fun, auto* obj)
    {
        handlers_[MessageType::id] = HandlerBinder<std::remove_pointer_t<decltype(obj)>, MessageType>(fun, obj);
    }

protected:

    using HandlerType = eul::function<void(const void*), 2*sizeof(void*)>;

    std::array<HandlerType, 255> handlers_;
};

} // namespace processor
} // namespace msgpu

