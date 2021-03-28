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

#include <cstddef>
#include <cstdint>

#include <array>

namespace modes 
{
namespace details
{
template <std::size_t bits_per_pixel>
constexpr std::size_t get_type_id()
{
    if constexpr (bits_per_pixel % 8 == 0)
    {
        return bits_per_pixel / 8;
    }
    return bits_per_pixel / 8 + 1;
}

template <std::size_t bytes> 
struct get_underlaying_type
{
};

template <>
struct get_underlaying_type<1>
{
    using type = uint8_t;
};

template <>
struct get_underlaying_type<2>
{
    using type = uint16_t;
};

template <>
struct get_underlaying_type<3>
{
    using type = uint16_t;
};


template <>
struct get_underlaying_type<4>
{
    using type = uint32_t;
};

template <std::size_t bits_per_pixel, typename T>
constexpr std::size_t pixels_in_type()
{
    return sizeof(T) * 8 / bits_per_pixel;
}

template <std::size_t bits_per_pixel, std::size_t current_mask = 0, std::size_t i = 0>
constexpr std::size_t get_mask()
{
    if constexpr (i == bits_per_pixel)
    {
        return current_mask;
    }
    else 
    {
        return get_mask<bits_per_pixel, (current_mask << 1) | 0x1, i + 1>();
    }
}

template <std::size_t width, std::size_t bits_per_pixel, typename T>
constexpr std::size_t get_size()
{
    if (width % pixels_in_type<bits_per_pixel, T>() == 0)
    {
        return width / pixels_in_type<bits_per_pixel, T>();
    }
    return width / pixels_in_type<bits_per_pixel, T>() + 1;
}

} // namespace details 

template <typename Type>
struct type_wrapper
{
    type_wrapper& operator=(uint8_t val)
    {
        *ptr &= ~(mask);
        *ptr |= (val << offset);
        return *this;  
    }

    template <typename T>
    operator T() const 
    {
        return ((*ptr) & mask) >> offset; 
    }

    Type* const ptr;
    const Type mask;
    const Type offset;
};


template <std::size_t width, std::size_t bits_per_pixel>
class LineBuffer
{
    using T = details::get_underlaying_type<details::get_type_id<bits_per_pixel>()>::type;
    constexpr static std::size_t pixels_in_type = details::pixels_in_type<bits_per_pixel, T>();
public:
    constexpr static T mask = details::get_mask<bits_per_pixel>();
    type_wrapper<T> operator[](std::size_t index)
    {
        const T offset = (index % pixels_in_type) * bits_per_pixel;
        const std::size_t position = (index * bits_per_pixel) / (sizeof(T) * 8);
        type_wrapper<T> wrapper{
            .ptr = &buf[position],
            .mask = mask << offset,
            .offset = offset
        };
        return wrapper;
    }

private:
    T buf[details::get_size<width, bits_per_pixel, T>()];
};

template <std::size_t width, std::size_t height, std::size_t bits_per_pixel>
class Buffer 
{
public: 
    using LineType = LineBuffer<width, bits_per_pixel>;
   
    LineType& operator[](const std::size_t index) 
    {
        return buffer_[index];
    }
private:
    LineType buffer_[height];
};
} // namespace modes

