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

#include <array>
#include <cstdio>

#include <eul/container/static_vector.hpp>
#include <eul/math/vector.hpp>

#include "mode/mode_base.hpp"

#include "messages/begin_primitives.hpp"
#include "messages/end_primitives.hpp"
#include "messages/write_vertex.hpp"

namespace msgpu::mode 
{

using Vec3 = eul::math::vector<float, 3>;
using Triangle = eul::container::static_vector<Vec3, 3>;

using Mesh = eul::container::static_vector<Triangle, 10000>;


template <typename Configuration>
class GraphicMode3D : public ModeBase<Configuration>
{
public:
    using Base = ModeBase<Configuration>;
    using ModeBase<Configuration>::ModeBase;
    using ModeBase<Configuration>::process;

    void clear() override
    {
        printf("Clearing mesh\n");
        mesh_.clear();
    }

    void process(const BeginPrimitives& msg)
    {
        printf("Begin primitives: %d\n", msg.type);
        if (mesh_.size() == mesh_.max_size())
        {
            printf("Mesh buffer is full, dropping primitive\n");
            return;
        }
        mesh_.push_back({});
    }

    void process(const EndPrimitives& )
    {
        printf("End primitives\n");
    }

    void process(const WriteVertex& v)
    {
        if (mesh_.back().size() == mesh_.back().max_size())
        {
            if (mesh_.size() == mesh_.max_size())
            {
                return;
            }
            mesh_.push_back({});
        }

        mesh_.back().push_back({v.x, v.y, v.z});
    }

    void render() override
    {
        printf("Render\n"); 
    }

protected:
    Mesh mesh_;
};

template <typename Configuration>
using DoubleBuffered3DGraphic = GraphicMode3D<Configuration>;

} // namespace msgpu::mode
