// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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
// You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <variant>
#include <span>

#include "generator/vga.hpp"

#include "modes/text/text_mode.hpp"
#include "modes/text/80x30_16.hpp"
#include "modes/text/40x30_16.hpp"
#include "modes/text/40x30_12bit.hpp"

#include <msgui/fonts/Font8x16.hpp>

namespace vga
{

enum class Modes
{
    Text_80x30_16,
    Text_40x30_16,
    Text_40x30_12bit,
    Graphic_640x480_16,
    Graphic_320x240_16,
    Graphic_320x240_12bit
};

std::string_view to_string(Modes mode);

class None
{
public:
    using type = void;
    void render()
    {
    }

    std::size_t fill_scanline(std::span<uint32_t> line, std::size_t line_number)
    {
        return 0;
    }

    std::span<uint16_t> get_line(std::size_t line)
    {
        return std::span<uint16_t>{};
    }

    void clear()
    {
    }
};

class Mode
{
public:
    Mode(vga::Vga& vga);

    using Text_80x30_16_8x16 = modes::text::PaletteTextMode<
        modes::text::Text_80x30_16color<msgui::fonts::Font8x16>>;

    using Text_40x30_16_8x16 = modes::text::PaletteTextMode<
        modes::text::Text_40x30_16color<msgui::fonts::Font8x16>>;

    using Text_40x30_12bit_8x16 = modes::text::TextMode<
        modes::text::Text_40x30_12bit<msgui::fonts::Font8x16>>;

    void clear();
    void switch_to(const Modes mode);
    void __time_critical_func(render)();
    void write(char c);
    void move_cursor(int row, int column);
    void set_cursor(int row, int column);
    void set_cursor_row(int row);
    void set_cursor_column(int column);
    void set_foreground_color(int foreground);
    void set_background_color(int background);
    void set_color(int foreground, int background);
    std::size_t __time_critical_func(fill_scanline)(std::span<uint32_t> line, std::size_t line_number);
    std::span<uint16_t> get_line(std::size_t line);
    
    using VariantType = std::variant<
        None, 
        Text_80x30_16_8x16,
        Text_40x30_16_8x16,
        Text_40x30_12bit_8x16
    >;
private:
    Vga& vga_;

    VariantType mode_;
   // Text_80x30_16_5x7 test_mode_;
};

} // namespace vga
