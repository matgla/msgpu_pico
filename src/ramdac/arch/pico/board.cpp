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
#include "board.hpp"

#include <cstdio>

#include <pico/stdlib.h>
#include <hardware/clocks.h>

#include "hal_dma.hpp"

#include "arch/pins_config.hpp"

namespace msgpu 
{

void initialize_uart() 
{
    uart_init(uart0, 230400);
    uart_set_hw_flow(uart0, false, false);
    uart_set_fifo_enabled(uart0, true);
    gpio_set_function(msgpu::uart_tx, GPIO_FUNC_UART);
    gpio_set_function(msgpu::uart_rx, GPIO_FUNC_UART);
}

void initialize_board()
{
    set_sys_clock_khz(250000, true);
    stdio_init_all();
    hal::enable_dma();
    initialize_uart();


    printf("Board initialized\n");
}


} // namespace msgpu
