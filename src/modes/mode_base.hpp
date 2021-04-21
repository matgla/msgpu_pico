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

#include <span>

#include <msgui/Position.hpp>

#include "generator/vga.hpp"

#include "modes/buffer.hpp"
#include "modes/mode_types.hpp"

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
    using type = std::array<std::array<uint16_t, Configuration::resolution_width>, Configuration::resolution_width>;
};

void vga_change_mode(const Modes mode);

template <typename Configuration> 
class ModeBase 
{
public:
    ModeBase()
    {
        vga_change_mode(Configuration::mode);
   }

   
    void base_render()
    {

    }

    std::size_t __time_critical_func(fill_scanline_buffer)(std::span<uint32_t> line, const uint16_t* scanline_buffer)
    {
        static uint32_t postamble[] = {
            0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
        };

        line[0] = 4;
        line[1] = host_safe_hw_ptr(line.data() + 8);
        line[2] = (Configuration::resolution_width - 4) / 2;
        line[3] = host_safe_hw_ptr(scanline_buffer + 4);
        line[4] = count_of(postamble);
        line[5] = host_safe_hw_ptr(postamble);
        line[6] = 0;
        line[7] = 0;

        line[8] = (scanline_buffer[0] << 16u) | COMPOSABLE_RAW_RUN;
        line[9] = (scanline_buffer[1] << 16u) | 0;
        line[10] = (COMPOSABLE_RAW_RUN << 16u) | scanline_buffer[2]; 
        line[11] = ((Configuration::resolution_width - 5) << 16u) | scanline_buffer[3];
        return 8;
    }

    void set_pixel(const msgui::Position position, const Configuration::Color color)
    {
        this->framebuffer_[position.y][position.x] = color;
    }


protected:
    using BufferType = BufferTypeGenerator<Configuration>::type;
    BufferType framebuffer_;
};

template <typename Configuration> 
class BufferedModeBase : public ModeBase<Configuration>
{
public: 

    void clear() 
    {
        this->framebuffer_.clear();
    }
 
    void __time_critical_func(copy_line_to_buffer)(std::size_t line_number, std::size_t next_line_id)
    {
        if (line_number < Configuration::resolution_height)
        {
            const auto& line = this->framebuffer_[line_number];
            auto& line_buffer = this->line_buffer_[next_line_id];
            for (int i = 0; i < line.size(); ++i)
            {
                const typename ModeBase<Configuration>::BufferType::PixelType colors = line.get(i);
                line_buffer[i] = Configuration::color_palette[colors & 0xf];
                line_buffer[++i] = Configuration::color_palette[(colors >> 4) & 0xf]; 
                line_buffer[++i] = Configuration::color_palette[(colors >> 8) & 0xf];
                line_buffer[++i] = Configuration::color_palette[(colors >> 12) & 0xf];
                line_buffer[++i] = Configuration::color_palette[(colors >> 16) & 0xf];
                line_buffer[++i] = Configuration::color_palette[(colors >> 20) & 0xf]; 
                line_buffer[++i] = Configuration::color_palette[(colors >> 24) & 0xf];
                line_buffer[++i] = Configuration::color_palette[(colors >> 28) & 0xf];

            }

        }
 
    }

     std::size_t __time_critical_func(fill_scanline)(std::span<uint32_t> line, std::size_t line_number)
    {
        const uint16_t* current_line = this->line_buffer_[line_number % this->line_buffer_size];
        std::size_t next_line_id = (line_number + 1) % this->line_buffer_size; 

        copy_line_to_buffer(line_number + 1, next_line_id);  

        return this->fill_scanline_buffer(line, current_line);
    }

    void __time_critical_func(base_render)() 
    {
        copy_line_to_buffer(0, 0);
    }

protected:
    constexpr static std::size_t line_buffer_size = 5;
    uint16_t line_buffer_[line_buffer_size][Configuration::resolution_width];
};

template <typename Configuration>
class NonBufferedModeBase : public ModeBase<Configuration>
{
public: 
     std::size_t __time_critical_func(fill_scanline)(std::span<uint32_t> line, std::size_t line_number)
    {
        const uint16_t* current_line = this->framebuffer_[line_number].data();

        return this->fill_scanline_buffer(line, current_line);
    }

    void clear() 
    {
        for (auto& line : this->framebuffer_)
        {
            line.fill(0);
        }
    }

    void set_pixel(const msgui::Position position, const Configuration::Color color)
    {
        this->framebuffer_[position.y][position.x] = color;
    }


};

} // namespace vga::modes
 
