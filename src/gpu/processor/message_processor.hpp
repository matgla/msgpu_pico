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

#include <array>

#include <eul/functional/function.hpp>

#include "io/message.hpp"

namespace msgpu 
{
namespace processor 
{

/// @brief Binds message handling method and object.
template <typename BindedType, typename MsgType>
struct HandlerBinder 
{
    typedef bool (BindedType::*fun)(const MsgType& payload);
    
    /// @brief Constructs binder object 
    ///
    /// @param f - pointer to member function for handling message 
    /// @param b - pointer to object on which member function will be called 
    HandlerBinder(fun f, BindedType* b) : f_(f), self_(b)
    {
    }

    /// @brief Calls handling function 
    ///
    /// @param payload - pointer to payload which will be converted to \ref MsgType 
    bool operator()(const void* payload) const
    {
        return (self_->*f_)(*static_cast<const MsgType*>(payload));
    }

private:
    fun f_;
    BindedType* self_;
};

/// @brief Registers handlers methods for message ids 
class MessageProcessor 
{
public: 
    /// @brief Constructs message processor (initializes data)
    MessageProcessor();

    /// @brief Process message received from io 
    ///
    /// @details Calls handler stored in \ref handlers_. 
    void process_message(const io::Message& message);
 
    /// @brief Register message processing function 
    ///
    /// @param fun - pointer to member function for handling message 
    /// @param obj - pointer to object on which member function will be called 
    template <typename MessageType>
    void register_handler(auto fun, auto* obj)
    {
        handlers_[MessageType::id] = HandlerBinder<std::remove_pointer_t<decltype(obj)>, MessageType>(fun, obj);
    }

protected:

    using HandlerType = eul::function<bool(const void*), 2*sizeof(void*)>;

    std::array<HandlerType, 255> handlers_;
};

} // namespace processor
} // namespace msgpu

