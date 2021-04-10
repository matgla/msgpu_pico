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

#include <tuple> 
#include <utility>

namespace processor
{

template <typename T, typename C>
class Handler
{
public:
    constexpr Handler(std::string_view name, C* object, T data) : handler_(name, std::make_pair(object, data))
    {
    }

    std::pair<std::string_view, std::pair<C*, T>> handler_;
};

template <typename... handlers>
class Handlers
{
public:
    Handlers(handlers... h) : handlers_(h...) 
    {
    }

    std::tuple<handlers...> handlers_;
};


template <typename... handlers>
Handlers(handlers...) -> Handlers<handlers...>;

} // namespace processor 

