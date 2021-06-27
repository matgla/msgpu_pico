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

#include "io/usart_point.hpp"

#include "hal_dma.hpp"

namespace msgpu::io 
{

constexpr uint8_t start_token = 0x7e;

std::optional<Message> UsartPoint::pop() 
{
    if (!messages_.empty())
    {
        if (messages_.front().received)
        {
            Message msg = messages_.front();
            messages_.pop_front();
            return msg;
        }
    }
    return {};
}

// ACTIONS 

void UsartPoint::prepare_for_header()
{
    hal::reset_dma_crc();
    messages_.push_back({});
    current_message_ = &messages_.back();
    hal::set_usart_dma_buffer(&current_message_->header, false);
    hal::set_usart_dma_transfer_count(sizeof(Header), true);
}

void UsartPoint::prepare_for_token()
{
    hal::set_usart_dma_buffer(&token_buffer_, false);
    hal::set_usart_dma_transfer_count(sizeof(token_buffer_), true);
}

void UsartPoint::prepare_for_payload()
{
    hal::reset_dma_crc();
    hal::set_usart_dma_buffer(current_message_->payload.data(), false);
    hal::set_usart_dma_transfer_count(current_message_->header.size, true);

}

void UsartPoint::prepare_for_crc()
{
    got_crc_ = static_cast<uint16_t>(hal::get_dma_crc());
    hal::set_usart_dma_buffer(&received_crc_, false);
    hal::set_usart_dma_transfer_count(sizeof(received_crc_), true);
}

void UsartPoint::store_message() 
{
    current_message_->received = true;
}

void UsartPoint::drop_message()
{
    messages_.pop_back();
}

// GUARDS 

bool UsartPoint::got_start_token()
{
    return token_buffer_ == start_token;
}

bool UsartPoint::verify_crc()
{
    return got_crc_ == received_crc_;
}

bool UsartPoint::message_without_size() 
{
    return current_message_->header.size == 0;
}

bool UsartPoint::check_header()
{
    return current_message_->header.size <= 32;
}

} // namespace msgpu::io
