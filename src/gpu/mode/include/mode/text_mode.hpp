/*
 *   Copyright (c) 2021 Mateusz Stadnik

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "mode/mode_base.hpp"

namespace msgpu::mode
{

template <typename Configuration, typename I2CType>
class TextMode : public ModeBase<Configuration, I2CType>
{
  public:
    TextMode(memory::VideoRam &framebuffer, memory::GpuRAM &gpuram, I2CType &i2c,
             io::UsartPoint &point)
        : ModeBase<Configuration, I2CType>(framebuffer, gpuram, i2c, point)
    {
    }

    void clear() override
    {
    }

    void render() override
    {
    }
};

} // namespace msgpu::mode
