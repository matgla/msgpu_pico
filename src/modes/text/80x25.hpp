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

#include <span>

#include <msgui/fonts/Font5x7.hpp>

#include <msgui/Position.hpp>

#include "generator/vga.hpp"

#include "modes/types.hpp"

namespace vga
{
namespace modes
{
namespace text
{

class Mode80x25
{
public:
    using type = Text;

    Mode80x25(Vga& vga);

    constexpr static int get_height()
    {
        return 25;
    }

    constexpr static int get_width()
    {
        return 80;
    }

    void write(uint8_t row, uint8_t column, char c);
    void write(const char c);

    void render_test_box();
    void render();

    void move_cursor(int row_offset, int column_offset);

    void set_cursor_row(int row);
    void set_cursor_column(int column);
    void set_cursor(int row, int column);
    void set_foreground_color(int foreground);
    void set_background_color(int background);
    void set_color(int foreground, int background);

    void fill_scanline(std::span<uint16_t> line, std::size_t line_number);
private:
    void set_pixel(msgui::Position position, int color);
    void render_font(const auto& bitmap, const int row, const int column, const int foreground, const int background);

    void clear_text_buffer();
    void clear_changed_bitmap(); 

    void add_pixel_to_render(int row, int column);
    void set_attribute(int row, int column, int attribute);
    uint16_t get_attribute(int row, int column) const;

    void render_screen();
    void render_cursor();

    char get_character(int row, int column) const;
    char *text_buffer_;
    uint8_t* changed_bitmap_;
    uint16_t* attributes_;

    int foreground_ = 63;
    int background_ = 0;

    uint8_t cursor_row_{0};
    uint8_t cursor_column_{0};

    bool draw_cursor_{false};
    bool cursor_visible_{false};

    constexpr static uint16_t time_to_blink_default = 30;
    uint16_t time_to_blink_{time_to_blink_default};
};

} // namespace text
} // namespace modes
} // namespace vga



