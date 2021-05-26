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

#include "config.hpp"
#include "generator/vga.hpp"
#include "modes/position.hpp"





namespace vga::modes 
{

template <typename Configuration, template <typename> typename Base> 
class PaletteModeBase : public Base<Configuration>
{
public: 
    void __time_critical_func(copy_line_to_buffer)(std::size_t line_number, std::size_t next_line_id)
    {
        if (line_number < Configuration::resolution_height)
        {
            const auto& line = this->get_readable_frame()[line_number];
            auto& line_buffer = this->line_buffer_[next_line_id];

            for (int i = 0; i < line.size(); ++i)
            {
                auto colors = line.get(i);
                line_buffer[i] = Configuration::color_palette[colors & 0xff];
                line_buffer[++i] = Configuration::color_palette[(colors >> 8) & 0xff]; 
                line_buffer[++i] = Configuration::color_palette[(colors >> 16) & 0xff];
                line_buffer[++i] = Configuration::color_palette[(colors >> 24) & 0xff];
              //  line_buffer[++i] = Configuration::color_palette[(colors >> 16) & 0xf];
              //  line_buffer[++i] = Configuration::color_palette[(colors >> 20) & 0xf]; 
              //  line_buffer[++i] = Configuration::color_palette[(colors >> 24) & 0xf];
              //  line_buffer[++i] = Configuration::color_palette[(colors >> 28) & 0xf];

            }

        }
 
    }

    std::size_t __time_critical_func(fill_scanline)(std::span<uint32_t> line, std::size_t line_number)
    {
        const uint16_t* current_line = this->line_buffer_[line_number % this->line_buffer_size];
        std::size_t next_line_id = (line_number + 1) % this->line_buffer_size; 

        copy_line_to_buffer(line_number + 1, next_line_id);  

        return vga::Vga::fill_scanline_buffer(line, std::span<const uint16_t>(current_line, Configuration::resolution_width));
    }

    void __time_critical_func(base_render)() 
    {
        Base<Configuration>::base_render();
        copy_line_to_buffer(0, 0);
    }

protected:
    constexpr static std::size_t line_buffer_size = 11;
    uint16_t line_buffer_[line_buffer_size][Configuration::resolution_width];
};

} // namespace vga::modes

