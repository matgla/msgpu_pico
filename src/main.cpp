// This file is part of MSGPU project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.  
#include <cstdio> 
#include <cstring> 

#include <unistd.h>

#include "modes/modes.hpp"

#include "processor/machine_interface.hpp"

#include "board.hpp"

#include "messages/begin_primitives.hpp"
#include "messages/header.hpp"
#include "messages/end_primitives.hpp"
#include "messages/clear_screen.hpp"
#include "messages/write_vertex.hpp"
#include "messages/set_perspective.hpp"
#include "messages/swap_buffer.hpp"

#include "qspi.hpp"

static vga::Mode mode; 
 
namespace msgpu 
{

std::size_t fill_scanline(std::span<uint32_t> buffer, std::size_t line)
{
    return mode.fill_scanline(buffer, line);
}

void frame_update() {
    mode.render();
}



} // namespace msgpu


static processor::MachineInterface proc(&mode, &msgpu::write_bytes);

void process_frame()
{
    proc.dma_run();
    proc.process_data();
}

int main() 
{
    msgpu::initialize_board();
 
    msgpu::initialize_signal_generator();
    
    msgpu::set_usart_handler([]{
        proc.schedule_update();
    });
    
    proc.schedule_update();
    while (true)
    {
        process_frame();
//        msgpu::sleep_ms(1000);
//        static int i = 0;
//        printf("Working %d\n", ++i);
    }

    msgpu::deinitialize_signal_generator();
} 


