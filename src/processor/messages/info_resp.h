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

#include <stdint.h>

#include "util.h"

extern "C" 
{

#define MAX_MODES 16

enum Mode
{
    Text = 0, 
    Graphic = 1
};

typedef struct packed
{
    uint8_t uses_color_palette : 1, 
            mode : 1,
            id : 6;
    uint16_t resolution_width;
    uint16_t resolution_height;
    uint16_t color_depth;
} mode_info;

typedef struct packed
{
    uint8_t version_major;
    uint8_t version_minor;
    mode_info modes[MAX_MODES];
} info_resp;

} // extern "C"
