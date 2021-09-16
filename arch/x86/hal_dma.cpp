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

#include <atomic>
#include <cstdint>
#include <cstring>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <future>
#include <optional>
#include <condition_variable>
#include <iostream>

#include <eul/crc/crc.hpp>

#include "board.hpp" 

namespace hal 
{

namespace 
{
static UsartHandler handler; 
static std::atomic<std::size_t> size_to_receive;
static std::atomic<void*> buffer_ptr;
static uint32_t crc;
static std::atomic<bool> trigger_;
static std::mutex mutex_;
static std::condition_variable cv;
static std::unique_ptr<std::thread> t;
static bool stop_usart = false;
} // namespace

void reset_dma_crc()
{
    crc = 0;
}

void set_usart_dma_buffer(void* buffer, bool trigger)
{
    {
    // std::unique_lock l(mutex_);
    buffer_ptr = buffer;
    trigger_ = trigger;
    } 
    cv.notify_one();
}

void set_usart_dma_transfer_count(std::size_t size, bool trigger)
{
    {
    // std::unique_lock l(mutex_);
    size_to_receive = size; 
    trigger_ = trigger;
    } 
    cv.notify_one();
}

void set_usart_handler(const UsartHandler& h)
{
    handler = h;
    stop_usart = false;
    t.reset(new std::thread([]{
        while (true)
        {
            //if (!trigger_)
            //{
            //    //std::this_thread::sleep_for(std::chrono::microseconds(100));
            //    continue;
            //}
            if (stop_usart) 
            {
                printf("Exit\n");
                return;
            }
            // std::unique_lock lk(mutex_);
            if (!trigger_)
            {
                continue;
                // if(!cv.wait_for(lk, std::chrono::microseconds(10), [] { return trigger_; }))
                // {
                    // continue;
                // }
            }
            trigger_ = false;
            std::vector<uint8_t> buf; 
            std::size_t i = 0;
            while (i < size_to_receive)
            {
                if (stop_usart) 
                {
                    printf("Exit 2\n");
                    return;
                }
                uint8_t byte = msgpu::read_byte();
                buf.push_back(byte);
                crc = calculate_crc<uint16_t, ccit_polynomial, 0, false>(std::span<const uint8_t>(&byte, 1), crc);
                ++i;
            }

            std::memcpy(buffer_ptr, buf.data(), size_to_receive);

            if (handler) 
            {
                crc = calculate_crc16(buf);
                // lk.unlock(); 
                handler();
            }
        }
    }));
    // t->detach();
}

void set_dma_mode(uint32_t mode)
{
}

uint32_t get_dma_crc()
{
    return crc;
}

void close_usart()
{
    std::cout << "Close uart" << std::endl;
    stop_usart = true;
}

} // namespace hal
