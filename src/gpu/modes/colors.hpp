// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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

namespace colors
{


// basic colors
constexpr static int red_1 = 0x01;
constexpr static int red_2 = 0x02;
constexpr static int red_3 = 0x03;

constexpr static int green_1 = 0x04;
constexpr static int green_2 = 0x08;
constexpr static int green_3 = 0x0c;

constexpr static int blue_1 = 0x10;
constexpr static int blue_2 = 0x20;
constexpr static int blue_3 = 0x30;

//

constexpr static int black = 0x00;
constexpr static int white = red_3 | green_3 | blue_3;

constexpr static int red = red_3;
constexpr static int green = green_3;
constexpr static int blue = blue_3;

constexpr static int yellow = red_3 | green_3;
constexpr static int magneta = red_3 | blue_3;
constexpr static int cyan = green_3 | blue_3;



} // namespace colors
