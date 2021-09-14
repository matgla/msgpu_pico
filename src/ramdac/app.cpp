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

#include "arch/hardware_config.hpp"
#include "arch/qspi_config.hpp"
#include "arch/pins_config.hpp"

#include "panic.hpp"

#include <board.hpp>

namespace msgpu 
{

App::App()
    : qspi_(framebuffer_config, 3.0f)
    , qspi_memory_(qspi_)
    , i2c_(i2c_slave_address, i2c_scl, i2c_sda)
    , vga_(generator::get_vga())
    , framebuffer_(qspi_memory_)
    , renderer_(vga_, framebuffer_)
{
}

void App::boot()
{
    printf("=========================\n");
    printf("|         RAMDAC        |\n");
    printf("=========================\n");

    printf("Booting procedure started.\n");

    qspi_.init();
    if (!qspi_memory_.init())
    {
        panic("FrameBuffer initialization failed\n");
    }

    if (!qspi_memory_.test())
    {
        panic("FrameBuffer memory test failed\n");
    }

    qspi_memory_.benchmark();

    printf("Initialize VGA generator\n");
    vga_.setup(&framebuffer_);
}

void App::run()
{
    uint8_t rx_buf[3];

    while (true)
    {
        // printf("Waiting for I2C\n");
        i2c_.read(rx_buf);
        // printf("Received data: ");
        // for (const auto byte : rx_buf)
        // {
            // printf("0x%x, ", byte);
        // }
        // printf("\n");

        switch (rx_buf[0])
        {
            case 0x01: 
            {
                printf ("Got data, reading 16 bytes\n");
            } break;
            case 0x02:
            {
                printf ("Got set mode command\n");
                renderer_.change_mode(static_cast<modes::Modes>(rx_buf[1]));
                printf ("New mode to set: 0x%x\n", rx_buf[1]);
            } break;
            case 0x03:
            {
                // printf ("Switch buffer to %d\n", rx_buf[1]);
                // msgpu::enable_dump();
                framebuffer_.block();
                framebuffer_.select_buffer(rx_buf[1], rx_buf[1]);
                uint8_t ack[2] = {0xac, 0x88};
                i2c_.write(ack);
                framebuffer_.unblock();
            }
        }
        // printf("\n");
    }
}

} // namespace msgpu

