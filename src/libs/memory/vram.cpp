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

#include "memory/vram.hpp"

#include <cstdio>

namespace msgpu::memory 
{

namespace 
{

constexpr std::size_t page_size = 1024;

} // namespace 

VideoRam::VideoRam(memory::QspiPSRAM& memory)
    : read_buffer_id_(0)
    , write_buffer_id_(1)
    , memory_(memory)
{
    mutex_init(&mutex_);
}

void VideoRam::set_resolution(uint16_t width, uint16_t height)
{
    block();
    width_ = width;
    height_ = height;
    unblock();
}

void VideoRam::set_color_space(uint8_t bits_per_pixel)
{
    block();
    bits_per_pixel_ = bits_per_pixel;
    unblock();
}

std::size_t VideoRam::get_address(uint8_t buffer_id, uint16_t line) const 
{
    return page_size * line + page_size * height_ * buffer_id;
}

void VideoRam::write_line(uint8_t buffer_id, uint16_t line, const ConstDataType<uint16_t> &data)
{
    const std::size_t address = get_address(buffer_id, line);

    // TODO: add compression 
    const ConstDataType<uint8_t> buffer(reinterpret_cast<const uint8_t*>(data.data()), data.size() * 2);

    memory_.acquire_bus();
    memory_.write(address, buffer); 
    memory_.wait_for_finish();
    memory_.release_bus();
}

void VideoRam::write_line(uint8_t buffer_id, uint16_t line, const ConstDataType<uint8_t> &data)
{
    const std::size_t address = get_address(buffer_id, line);

    memory_.acquire_bus();
    memory_.write(address, data);
    // TODO: in future move at beginning of functions to be 'async'
    memory_.wait_for_finish();
    memory_.release_bus();
}

void VideoRam::read_line(uint8_t buffer_id, uint16_t line, DataType<uint16_t> data)
{
    const std::size_t address = get_address(buffer_id, line); 
    const DataType<uint8_t> buffer(reinterpret_cast<uint8_t*>(data.data()), data.size() * 2);

    memory_.acquire_bus();
    memory_.read(address, buffer); 
    memory_.wait_for_finish();
    memory_.release_bus();
}

void VideoRam::read_line(uint8_t buffer_id, uint16_t line, DataType<uint8_t> data)
{
    const std::size_t address = get_address(buffer_id, line); 

    memory_.acquire_bus();
    memory_.read(address, data);
    memory_.wait_for_finish();
    memory_.release_bus();
}

void VideoRam::write_line(uint16_t line, const ConstDataType<uint16_t> &data)
{
    mutex_enter_blocking(&mutex_);
    const uint8_t id = write_buffer_id_;
    write_line(id, line, data);
    mutex_exit(&mutex_);
}

void VideoRam::write_line(uint16_t line, const ConstDataType<uint8_t> &data)
{
    mutex_enter_blocking(&mutex_);
    const uint8_t id = write_buffer_id_;
    write_line(id, line, data);
    mutex_exit(&mutex_);
}

void VideoRam::read_line(uint16_t line, DataType<uint16_t> data)
{
    mutex_enter_blocking(&mutex_);
    const uint8_t id = write_buffer_id_;
    read_line(id, line, data);
    mutex_exit(&mutex_);
}

void VideoRam::read_line(uint16_t line, DataType<uint8_t> data)
{
    mutex_enter_blocking(&mutex_);
    const uint8_t id = write_buffer_id_;
    read_line(id, line, data);
    mutex_exit(&mutex_);
}


void VideoRam::select_buffer(uint8_t read_buffer_id, uint8_t write_buffer_id)
{
    mutex_enter_blocking(&mutex_);
    read_buffer_id_ = read_buffer_id;
    write_buffer_id_ = write_buffer_id;
    mutex_exit(&mutex_);
}

uint8_t VideoRam::get_read_buffer_id() 
{
    mutex_enter_blocking(&mutex_);
    const uint8_t ret = read_buffer_id_;
    mutex_exit(&mutex_);
    return ret;
}

uint8_t VideoRam::get_write_buffer_id() 
{
    mutex_enter_blocking(&mutex_);
    const uint8_t ret = write_buffer_id_;
    mutex_exit(&mutex_);
    return ret;
}

void VideoRam::block()
{
}

void VideoRam::unblock()
{
}

} // namespace msgpu::memory
