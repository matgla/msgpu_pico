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
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <variant>
#include <span>

#include "generator/vga.hpp"
#include "modes/text/80x25.hpp"

namespace vga
{

enum class Modes
{
    Text_80x25 = 0,
    Graphic_256x240 = 1
};

std::string_view to_string(Modes mode);

class None
{
public:
    using type = void;
    void render()
    {
    }

    void fill_scanline(std::span<uint16_t> line, std::size_t line_number)
    {
    }
};

class Mode
{
public:
    Mode(vga::Vga& vga);

    void switch_to(const Modes mode);
    void render();
    void write(char c);
    void move_cursor(int row, int column);
    void set_cursor(int row, int column);
    void set_cursor_row(int row);
    void set_cursor_column(int column);
    void set_foreground_color(int foreground);
    void set_background_color(int background);
    void set_color(int foreground, int background);
    void fill_scanline(std::span<uint16_t> line, std::size_t line_number);
private:
    std::variant<None
        , vga::modes::text::Mode80x25
        > mode_;

    Vga& vga_;
};

} // namespace vga
