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

#include <cstdio>

#include "mode/mode_base.hpp"

#include "messages/begin_primitives.hpp"
#include "messages/end_primitives.hpp"
#include "messages/write_vertex.hpp"

namespace msgpu::mode 
{

template <typename Configuration, std::size_t BufferSize>
class GraphicMode3D : public ModeBase<Configuration, BufferSize>
{
public:
    using Base = ModeBase<Configuration, BufferSize>;
    using ModeBase<Configuration, BufferSize>::process;

    void process(const BeginPrimitives& )
    {
        printf("Begin primitives\n");
    }

    void process(const EndPrimitives& )
    {
        printf("End primitives\n");
    }

    void process(const WriteVertex& )
    {
    }

};

template <typename Configuration>
using DoubleBuffered3DGraphic = GraphicMode3D<Configuration, 2>;

} // namespace msgpu::mode
