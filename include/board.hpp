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

namespace msgpu 
{

void initialize_board();

void initialize_signal_generator();
void deinitialize_signal_generator();

void fill_scanline(std::span<uint32_t> buffer, std::size_t line);

uint8_t read_byte();
void write_bytes(std::span<uint8_t> byte);

} // namespace msgpu  

