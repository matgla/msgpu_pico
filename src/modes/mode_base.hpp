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

#include <cstdio>
#include <span>

#include <msgui/Position.hpp>

#include "generator/vga.hpp"

#include "modes/buffer.hpp"
#include "modes.hpp"

#include "board.hpp"
#include "sync.hpp"

namespace vga::modes 
{

template <typename Configuration, bool uses_color_palette>
struct BufferTypeGeneratorImpl;

template <typename Configuration>
struct BufferTypeGenerator
{
    using type = typename BufferTypeGeneratorImpl<Configuration, Configuration::uses_color_palette>::type;
};

template <typename Configuration> 
struct BufferTypeGeneratorImpl<Configuration, true> 
{
    using type = modes::Buffer<Configuration::resolution_width, Configuration::resolution_height, Configuration::bits_per_pixel>;
};

template <typename Configuration>
struct BufferTypeGeneratorImpl<Configuration, false>
{
    using type = std::array<std::array<typename Configuration::ColorType, Configuration::resolution_width>, Configuration::resolution_width>;
};

template <typename Configuration> 
class ModeBase 
{
public:
    ModeBase()
    {
        mutex_init(&mutex_);
        get_vga().change_mode(Configuration::mode);
    }

    void clear()
    {
        mutex_enter_blocking(&mutex_);
        for (auto& line : get_writable_frame())
        {
            line.fill(0);
        }
        mutex_exit(&mutex_);
    }

    void base_render()
    {

    }

    void __time_critical_func(set_pixel)(const msgui::Position position, const typename Configuration::Color color)
    {
        if (position.x < 0 || position.x >= Configuration::resolution_width)
        {
            return;
        }

        if (position.y < 0 || position.y >= Configuration::resolution_height)
        {
            return;
        }


        this->framebuffer_[position.y][position.x] = color;
    }

    using BufferType = typename BufferTypeGenerator<Configuration>::type;

    BufferType& get_writable_frame()
    {
        return framebuffer_;
    }

    const BufferType& get_readable_frame() const 
    {
        return framebuffer_;
    }

protected:
    mutex_t mutex_;
    BufferType framebuffer_;
};


template <typename Configuration, template <typename> typename Base>
class RawModeBase : public Base<Configuration>
{
public: 
    std::size_t __time_critical_func(fill_scanline)(std::span<uint32_t> line, std::size_t line_number)
    {
        if (line_number >= Configuration::resolution_height)
        {
            return 0;
        }
        mutex_enter_blocking(&this->mutex_);
        const uint16_t* current_line = this->get_readable_frame()[line_number].data();
        mutex_exit(&this->mutex_);
        return vga::Vga::fill_scanline_buffer(line, std::span<const uint16_t>(current_line, Configuration::resolution_width));
    }

    void __time_critical_func(base_render)() 
    {
        Base<Configuration>::base_render();
    }


    void __time_critical_func(clear)() 
    {
        Base<Configuration>::clear();
    }

    void __time_critical_func(set_pixel)(const msgui::Position position, const typename Configuration::Color color)
    {
        if (position.x < 0 || position.x >= Configuration::resolution_width)
        {
            return;
        }

        if (position.y < 0 || position.y >= Configuration::resolution_height)
        {
            return;
        }

        mutex_enter_blocking(&this->mutex_);
        this->get_writable_frame()[position.y][position.x] = 0xfff;//color;
        mutex_exit(&this->mutex_);
    }
};

template <typename Configuration>
using SingleBufferedPaletteBase = PaletteModeBase<Configuration, ModeBase>;

template <typename Configuration>
using SingleBufferedRawBase = RawModeBase<Configuration, ModeBase>;

} // namespace vga::modes
 
