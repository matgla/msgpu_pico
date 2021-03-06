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

#include <cstdint> 

#include <gmock/gmock.h>

class HalDmaInterface 
{
public: 
    virtual ~HalDmaInterface() = default;

    virtual void set_usart_dma_buffer(void* buffer, bool trigger) = 0;
    virtual void set_usart_dma_transfer_count(std::size_t size, bool trigger) = 0;
    virtual void reset_dma_crc() = 0;
    virtual uint32_t get_dma_crc() = 0;
};

class HalDmaMock : public HalDmaInterface
{
public: 
    MOCK_METHOD2(set_usart_dma_buffer, void(void*, bool trigger));
    MOCK_METHOD2(set_usart_dma_transfer_count, void(std::size_t size, bool trigger));
    MOCK_METHOD0(reset_dma_crc, void());
    MOCK_METHOD0(get_dma_crc, uint32_t());
};
