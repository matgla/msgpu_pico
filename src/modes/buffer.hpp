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

namespace vga 
{
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

    using SelfType = LineBuffer<width, bits_per_pixel>;

    constexpr static std::size_t pixels_in_type = details::pixels_in_type<bits_per_pixel, T>();
    
    using DataType = std::array<T, details::get_size<width, bits_per_pixel, T>()>;
public:

    using PixelType = T;
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

    struct iterator 
    {
        iterator(std::size_t pos, SelfType& self) 
            : pos_(pos)
            , self_(self)
        {
        }

        iterator operator++(int) 
        {
            iterator prev = *this; 
            pos_ += 1; 
            return prev;
        }
        
        iterator& operator++()
        {
            ++pos_;
            return *this;
        }

        type_wrapper<T> operator*()
        {
            return self_[pos_]; 
        }

        bool operator==(const iterator& it) const
        {
            return it.pos_ == pos_; 
        }

        bool operator!=(const iterator& it) const 
        {
            return it.pos_ != pos_;
        }

        private:
            std::size_t pos_;
            SelfType& self_;
    };

    iterator begin() 
    {
        return iterator(0, *this);
    }

    iterator end()
    {
        return iterator(width, *this);
    }

    void clear()
    {
        buf.fill(0);
    }
private:
    DataType buf;
};

template <std::size_t width, std::size_t height, std::size_t bits_per_pixel>
class Buffer 
{
public: 
    using LineType = LineBuffer<width, bits_per_pixel>;
    using PixelType = LineType::PixelType;

    LineType& operator[](const std::size_t index) 
    {
        return buffer_[index];
    }

    void clear()
    {
        for (auto& line : buffer_)
        {
            line.clear();
        }
    }
    constexpr static std::size_t color_depth = details::get_mask<bits_per_pixel>();
private:
    std::array<LineType, height> buffer_;
};

} // namespace modes
} // namespace vga 

