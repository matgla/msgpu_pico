// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <variant>
#include <span>

#include <eul/math/matrix.hpp>

#include "modes/text/text_mode.hpp"
#include "modes/text/80x30_16.hpp"
#include "modes/text/40x30_16.hpp"
#include "modes/text/40x30_12bit.hpp"

#include "modes/graphic/graphic_mode.hpp"
#include "modes/graphic/320x240_12bit.hpp"
#include "modes/graphic/320x240_256.hpp"
#include <msgui/fonts/Font8x16.hpp>

#include "messages/begin_primitives.hpp"

namespace vga
{

class ConfigurationNone 
{
public:
    constexpr static bool double_buffered = false;
};

class None
{
public:
    using type = void;
    using ConfigurationType = ConfigurationNone;
    void render()
    {
    }

    std::size_t fill_scanline(std::span<uint32_t> line, std::size_t line_number)
    {
        static_cast<void>(line);
        static_cast<void>(line_number);
        return 0;
    }

    void clear()
    {
    }
};

class Mode
{
public:
    Mode();

    using Text_80x30_16_8x16 = modes::text::PaletteTextMode<
        modes::text::Text_80x30_16color<msgui::fonts::Font8x16>>;

    using Text_40x30_16_8x16 = modes::text::PaletteTextMode<
        modes::text::Text_40x30_16color<msgui::fonts::Font8x16>>;

    using Text_40x30_12bit_8x16 = modes::text::TextMode<
        modes::text::Text_40x30_12bit<msgui::fonts::Font8x16>>;

    using Graphic_320x240_12bit = modes::graphic::GraphicMode<
        modes::graphic::Graphic_320x240_12bit>;

    using Graphic_320x240_256 = modes::graphic::PaletteGraphicMode<
        modes::graphic::Graphic_320x240_256>;


    using ModeTypes = std::tuple< 
        Text_80x30_16_8x16,
        Text_40x30_16_8x16,
        Text_40x30_12bit_8x16,
        Graphic_320x240_12bit,
        Graphic_320x240_256
    >; 

    void clear();
    void switch_to(const vga::modes::Modes mode);
    void __time_critical_func(render)();
    void write(char c);
    void move_cursor(int row, int column);
    void set_cursor(int row, int column);
    void set_cursor_row(int row); void set_cursor_column(int column);
    void set_foreground_color(int foreground);
    void set_background_color(int background);
    void set_color(int foreground, int background);
    std::size_t __time_critical_func(fill_scanline)(std::span<uint32_t> line, std::size_t line_number);
    void set_pixel(int x, int y, uint16_t color);
    void draw_line(int x1, int y1, int x2, int y2);

    void swap_buffer();
    // 3d api 
    void begin_primitives(PrimitiveType type);
    void end_primitives();
    void write_vertex(float x, float y, float z);

    void set_perspective(float angle, float aspect, float z_far, float z_near);

    using VariantType = std::variant<
        None, 
        Text_80x30_16_8x16,
        Text_40x30_16_8x16,
        Text_40x30_12bit_8x16,
        Graphic_320x240_12bit,
        Graphic_320x240_256
    >;

private:
    
    static inline VariantType mode_;
};

} // namespace vga
