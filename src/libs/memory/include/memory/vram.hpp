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

#include "memory/psram.hpp"

namespace msgpu::memory 
{

class VideoRam
{
public:
    template <typename T>
    using ConstDataType = std::span<const T>;

    template <typename T>
    using DataType = std::span<T>;

    VideoRam(memory::QspiPSRAM& memory);

    void set_resolution(uint16_t width, uint16_t height);
    void set_color_space(uint8_t bits_per_pixel);

    void write_line(uint16_t line, const ConstDataType<uint16_t>& data);
    void write_line(uint16_t line, const ConstDataType<uint8_t>& data);

    void read_line(uint16_t line, DataType<uint16_t> data);
    void read_line(uint16_t line, DataType<uint8_t> data);

    void select_buffer(uint8_t buffer_id);
    uint8_t get_current_buffer_id() const;
    std::size_t get_address(uint16_t line) const;
private:
    uint8_t bits_per_pixel_;
    uint8_t buffer_id_;
    uint16_t width_;
    uint16_t height_;
    memory::QspiPSRAM& memory_;
};

} // namespace msgpu::memory

