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

namespace msgpu::mode 
{

template <typename Configuration, bool uses_color_palette>struct BufferTypeGeneratorImpl;

template <typename Configuration>
struct BufferTypeGenerator 
{
    using type = typename BufferTypeGeneratorImpl<Configuration, Configuration::uses_color_palette>::type;
};

template <typename Configuration> 
struct BufferTypeGeneratorImpl<Configuration, true> 
{
    using type = modes::Buffer<Configuration::resolution_width, Configuration::resolution_height, Configuration::bits_per_pixel>;
};

template <typename Configuration>
struct BufferTypeGeneratorImpl<Configuration, false>
{
    using type = std::array<std::array<typename Configuration::ColorType, Configuration::resolution_width>, Configuration::resolution_width>;
};

t

} // namespace msgpu::mode

