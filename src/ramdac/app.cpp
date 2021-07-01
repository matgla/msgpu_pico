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

#include "app.hpp"

#include <cstdio>

#include "arch/qspi_config.hpp"

#include "core/panic.hpp"

namespace msgpu 
{

App::App()
    : qspi_(framebuffer_config, 3.0f)
    , framebuffer_(qspi_)
{
}

void App::boot()
{
    printf("=========================\n");
    printf("|         RAMDAC        |\n");
    printf("=========================\n");

    printf("Booting procedure started.\n");

    qspi_.init();
    if (!framebuffer_.init())
    {
        panic("FrameBuffer initialization failed\n");
    }

    if (!framebuffer_.test())
    {
        panic("FrameBuffer memory test failed\n");
    }

    framebuffer_.benchmark();
}

} // namespace msgpu

