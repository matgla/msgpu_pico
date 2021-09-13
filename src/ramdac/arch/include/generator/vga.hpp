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
#include <span> 

#include "modes.hpp"

#include "config.hpp"
#include "sync.hpp"

#include "memory/vram.hpp"

namespace msgpu::generator
{

class Vga
{
public:
    Vga(modes::Modes mode);
    
    void change_mode(modes::Modes mode);

    void setup(memory::VideoRam* vram);

    std::size_t display_line(std::size_t line, std::span<uint32_t> to_display);

    void block();
    void unblock();
private:
    mutex_t vga_mutex_;
    memory::VideoRam* vram_;
};

Vga& get_vga();

} // namespace msgpu::generator

