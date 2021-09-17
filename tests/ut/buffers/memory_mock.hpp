// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it is under the terms of the GNU General Public License as published by
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

#include <cstdint>

#include <gmock/gmock.h>

namespace msgpu::mocks
{

class MemoryMock
{
  public:
    MOCK_METHOD(std::size_t, write, (std::size_t, const void *, std::size_t));
    MOCK_METHOD(std::size_t, read, (std::size_t, void *, std::size_t));
};

} // namespace msgpu::mocks
