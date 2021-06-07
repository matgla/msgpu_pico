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

#include <eul/functional/function.hpp>

namespace hal 
{

using UsartHandler = eul::function<void(), sizeof(void*)>;

void reset_dma_crc();
void set_usart_dma_buffer(void* buffer, bool trigger);
void set_usart_dma_transfer_count(std::size_t size, bool trigger);
void set_usart_handler(const UsartHandler& handler);

void set_dma_mode(uint32_t mode);

uint32_t get_dma_crc();

void enable_dma();

}
