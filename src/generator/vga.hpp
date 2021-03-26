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

#include <string_view>

#include "generator/timings.hpp"

#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>

namespace vga
{

class Vga
{
public:
    Vga(const scanvideo_mode_t* mode);
    
    struct Config
    {
    };
   

    void setup(const Config& config);
    bool is_vsync() const;
    bool render() const;
    void render(bool enable);

private:
    const scanvideo_mode_t* mode_;
};

} // namespace vga
