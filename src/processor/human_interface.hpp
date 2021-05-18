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

#include <cstdint>
#include <string_view>
#include <tuple>

#include "modes/modes.hpp"

namespace processor
{

class HumanInterface
{
public:
    using WriteCallback = void(*)(std::span<const uint8_t>);
    HumanInterface(vga::Mode& mode, WriteCallback write_callback);

    void process_data();
    void process(uint8_t byte);
    void dma_run();
private:
    void process_write(uint8_t byte);
    void process_command();
    std::string_view get_next_part();

    void write();
    void help();
    void mode();
    void clear();
    void set_color();

    enum class State 
    {
        waiting_for_command, 
        writing
    };

    char buffer_[100];
    std::string_view to_parse_;
    std::size_t position_;
    vga::Mode* mode_;
    bool escape_code_ = false;
    bool cursor_move_ = false;
    State state_;
    WriteCallback write_;
};

} // namespace processor
