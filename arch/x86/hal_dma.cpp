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
//

#include "hal_dma.hpp"

#include <cstdint>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

#include <eul/crc/crc.hpp>

#include "board.hpp" 

namespace hal 
{

namespace 
{
static UsartHandler handler; 
static std::size_t size_to_receive;
static void* buffer_ptr;
static uint32_t crc;
static bool trigger_;
static std::mutex mutex_;
static std::condition_variable cv;
} // namespace

void reset_dma_crc()
{
    crc = 0;
}

void set_usart_dma_buffer(void* buffer, bool trigger)
{
    {
    std::unique_lock l(mutex_);
    buffer_ptr = buffer;
    trigger_ = trigger;
    } 
    cv.notify_one();
}

void set_usart_dma_transfer_count(std::size_t size, bool trigger)
{
    {
    std::unique_lock l(mutex_);
    size_to_receive = size; 
    trigger_ = trigger;
    } 
    cv.notify_one();
}

void set_usart_handler(const UsartHandler& h)
{
    handler = h;
    static std::thread t([]{
        while (true)
        {
            std::unique_lock lk(mutex_);
            if (!trigger_)
                cv.wait(lk, [] { return trigger_; });
            trigger_ = false;
            std::vector<uint8_t> buf; 
            std::size_t i = 0;
            while (i < size_to_receive)
            {
                uint8_t byte = msgpu::read_byte();
                buf.push_back(byte);
                crc = calculate_crc<uint16_t, ccit_polynomial, 0, false>(std::span<const uint8_t>(&byte, 1), crc);
                ++i;
            }

            std::memcpy(buffer_ptr, buf.data(), size_to_receive);

            if (handler) 
            {
                crc = calculate_crc16(buf);
                lk.unlock(); 
                handler();
            }
        }
    });
}

void set_dma_mode(uint32_t mode)
{
}

uint32_t get_dma_crc()
{
    return crc;
}

} // namespace hal
