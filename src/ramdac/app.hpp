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

#include "arch/i2c.hpp"
#include "qspi.hpp"

#include "generator/vga.hpp"
#include "memory/psram.hpp"
#include "memory/vram.hpp"
#include "renderer/renderer.hpp"

namespace msgpu 
{

class App 
{
public: 
    App();

    void boot();
    void run();
private:
    bool init_framebuffer();

    Qspi qspi_;
    memory::QspiPSRAM qspi_memory_;
    I2C i2c_;
    generator::Vga& vga_;
    memory::VideoRam framebuffer_;
    renderer::Renderer renderer_;
};

} // namespace msgpu
