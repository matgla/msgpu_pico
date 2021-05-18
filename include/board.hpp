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
#include <span>

#include <eul/functional/function.hpp>

namespace msgpu 
{

void initialize_board();

void initialize_signal_generator();
void deinitialize_signal_generator();

std::size_t fill_scanline(std::span<uint32_t> buffer, std::size_t line);

void frame_update();

uint8_t read_byte();
void write_bytes(std::span<uint8_t> byte);

void set_resolution(uint16_t width, uint16_t height);

uint32_t get_millis();
void sleep_ms(uint32_t time);

using UsartHandler = eul::function<void(), sizeof(void*)>;

void set_usart_dma_buffer(uint8_t* buffer, bool trigger);
void set_usart_dma_transfer_count(std::size_t size, bool trigger);
void set_usart_handler(const UsartHandler& handler);

} // namespace msgpu  

void start_vga();
