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

#include <cstdint>

enum class Polarity
{
    Positive,
    Negative
};

struct Timings
{
    float pixel_frequency;
    struct Line
    {
        const uint32_t visible_pixels;
        const uint32_t front_porch_pixels;
        const uint32_t sync_pulse_pixels;
        const uint32_t back_porch_pixels;
    };

    struct Frame
    {
        const uint32_t visible_lines;
        const uint32_t front_porch_lines;
        const uint32_t sync_pulse_lines;
        const uint32_t back_porch_lines;
    };

    const Line line;
    const Frame frame;
    const Polarity hsync_polarity;
    const Polarity vsync_polarity;
};

constexpr Timings svga_800x600_60 = {
    .pixel_frequency = 40.0,
    .line = {
        .visible_pixels = 800,
        .front_porch_pixels = 40,
        .sync_pulse_pixels = 128,
        .back_porch_pixels = 88
    },
    .frame = {
        .visible_lines = 600,
        .front_porch_lines = 1,
        .sync_pulse_lines = 4,
        .back_porch_lines = 23
    },
    .hsync_polarity = Polarity::Positive,
    .vsync_polarity = Polarity::Negative
};
