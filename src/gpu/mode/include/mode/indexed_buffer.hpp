/*
 *   Copyright (c) 2021 Mateusz Stadnik

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>
#include <bitset>
#include <cstdint>

namespace msgpu::mode
{

template <typename T, std::size_t N, typename IndexType = std::size_t>
class IndexedBuffer
{
  public:
    IndexType allocate()
    {
        for (IndexType i = 0; i < N; ++i)
        {
            if (!map_.test(i))
            {
                map_[i] = 1;
                return i;
            }
        }
        return static_cast<IndexType>(-1);
    }

    void deallocate(IndexType id)
    {
        if (id >= map_.size())
        {
            return;
        }
        if (map_.test(id))
        {
            map_[id] = 0;
        }
    }

    bool write(IndexType id, const T &data)
    {
        if (id >= map_.size())
        {
            return false;
        }

        if (!map_.test(id))
        {
            return false;
        }

        data_[id] = data;
        return true;
    }

    const T &operator[](IndexType id) const
    {
        return data_[id];
    }

    T &operator[](IndexType id)
    {
        return data_[id];
    }

    bool test(IndexType t) const
    {
        if (t >= N)
        {
            return false;
        }
        return map_.test(t);
    }

    constexpr IndexType size() const
    {
        return N;
    }

  private:
    std::bitset<N> map_;
    std::array<T, N> data_;
};

} // namespace msgpu::mode
