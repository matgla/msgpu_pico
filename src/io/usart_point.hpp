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
#include <cstdint>

#include <boost/sml.hpp>

#include "messages/header.hpp"

namespace msgpu::io 
{

struct init{};
struct data{};

/// @brief Implements external world interface with standard USART protocol
/// @author Mateusz Stadnik
class UsartPoint
{
    using Self = UsartPoint;
public:
    auto operator()() 
    {
        using namespace boost::sml;

        return make_transition_table(
            *"init"_s + event<init> / (&Self::prepare_for_token) = "wait_for_start_token"_s
        );
    }
private:
    void prepare_for_header();
    void prepare_for_token();

    uint8_t token_buffer_;
    uint16_t received_crc_;
    uint16_t calculated_crc_;
};

} // namespace msgpu::io 

