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

#include "config.hpp"

namespace vga 
{
namespace modes 
{
namespace details
{

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
    type_wrapper& __time_critical_func(operator=)(uint8_t val)
    {
        *ptr &= ~(mask);
        *ptr |= (val << offset);
        return *this;  
    }
    

    template <typename T>
    __time_critical_func(operator T()) const 
    {
        return ((*ptr) & mask) >> offset; 
    }

    Type* const ptr;
    const Type mask;
    const Type offset;
};

template <typename Type>
struct const_type_wrapper
{
    template <typename T>
    __time_critical_func(operator T()) const 
    {
        return ((*ptr) & mask) >> offset; 
    }

    const Type* const ptr;
    const Type mask;
    const Type offset;
};


template <std::size_t width, std::size_t bits_per_pixel>
class LineBuffer
{
    using T = uint32_t;

    using SelfType = LineBuffer<width, bits_per_pixel>;

    constexpr static std::size_t pixels_in_type = details::pixels_in_type<bits_per_pixel, T>();
    
    using DataType = std::array<T, details::get_size<width, bits_per_pixel, T>()>;
public:

    using PixelType = T;
    constexpr static T mask = details::get_mask<bits_per_pixel>();

    type_wrapper<T> __time_critical_func(operator[])(std::size_t index)
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
    
    T __time_critical_func(get)(std::size_t index) const
    {
        const std::size_t position = (index * bits_per_pixel) / (sizeof(T) * 8);

        return buf[position]; 
    }
    const_type_wrapper<T> __time_critical_func(operator[])(std::size_t index) const 
    {
        const T offset = (index % pixels_in_type) * bits_per_pixel;
        const std::size_t position = (index * bits_per_pixel) / (sizeof(T) * 8);
        const_type_wrapper<T> wrapper{
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

        iterator __time_critical_func(operator++)(int) 
        {
            iterator prev = *this; 
            pos_ += 1; 
            return prev;
        }
        
        iterator& __time_critical_func(operator++)()
        {
            ++pos_;
            return *this;
        }

        iterator __time_critical_func(operator-)(int decr)
        {
            return iterator(pos_ - decr, self_);
        }

        type_wrapper<T> __time_critical_func(operator*)()
        {
            return self_[pos_]; 
        }

        bool __time_critical_func(operator==)(const iterator& it) const
        {
            return it.pos_ == pos_; 
        }

        bool __time_critical_func(operator!=)(const iterator& it) const 
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

    constexpr static std::size_t size()
    {
        return width;
    }
    void clear()
    {
        buf.fill(0);
    }
    void fill(T d)
    {
        buf.fill(d);
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

    LineType& __time_critical_func(operator[])(const std::size_t index) 
    {
        return buffer_[index];
    }

    const LineType& __time_critical_func(operator[])(const std::size_t index) const
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

    LineType* begin() 
    {
        return &buffer_[0];
    }

    LineType* end()
    {
        return  &buffer_[height];
    }

    const LineType* begin() const 
    {
        return &buffer_[0];
    }

    const LineType* end() const
    {
        return  &buffer_[height];
    }

    constexpr static std::size_t size() 
    {
        return height;
    }
    constexpr static std::size_t color_depth = details::get_mask<bits_per_pixel>();
private:
    LineType buffer_[height];
};

} // namespace modes
} // namespace vga 

