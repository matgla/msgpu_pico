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

#include "renderer.hpp"

#include <cstdio>

namespace msgpu::renderer 
{

Renderer::Renderer(generator::Vga& vga)
    : vga_(vga) 
{
}

bool Renderer::change_mode(modes::Modes mode)
{
    printf("Change rendering mode to: %s\n", modes::to_string(mode));
    vga_.change_mode(mode);

    return true;
}

} // namespace msgpu::renderer
