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

#include "board.hpp" 

#include <cstdio>

#include "app.hpp"

namespace msgpu 
{

// TODO: to be removed
void frame_update()
{
}


// TODO: to be removed 
std::span<const uint8_t> get_scanline(std::size_t line) 
{
    uint8_t buf[640] = {};
    static_cast<void>(line);
    return std::span<const uint8_t>(buf);
}

} // namespace msgpu

int main() 
{
    msgpu::initialize_board();
    
    msgpu::App app;
    app.boot();
    app.run();

    while (true)
    {
    }
}
