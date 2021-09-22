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

#include <bitset>
#include <cstdint>

namespace msgpu::buffers
{

template <std::size_t N>
class IdGenerator
{
  public:
    uint32_t allocate_name()
    {
        for (uint32_t i = 0; i < N; ++i)
        {
            if (names_map_[i] == 0)
            {
                names_map_[i] = 1;
                return i;
            }
        }
        return 0xffffffff;
    }

    void release_name(uint32_t name)
    {
        if (name >= N)
            return;
        names_map_[name] = 0;
    }

    bool test(uint32_t name) const
    {
        if (name >= N)
            return false;
        return names_map_[name];
    }

  protected:
    std::bitset<N> names_map_;
};

} // namespace msgpu::buffers

