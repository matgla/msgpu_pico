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

#include <boost/sml.hpp>

#include "processor/message_processor.hpp"

#include "board.hpp"
#include "hal_dma.hpp"

#include "messages/change_mode.hpp"
#include "messages/set_pixel.hpp"
#include "messages/draw_line.hpp"
#include "messages/info_req.hpp"
#include "messages/clear_screen.hpp"
#include "messages/begin_primitives.hpp"
#include "messages/end_primitives.hpp"
#include "messages/write_vertex.hpp"
#include "messages/write_text.hpp"
#include "messages/set_perspective.hpp"
#include "messages/swap_buffer.hpp"
#include "messages/draw_triangle.hpp"

#include "messages/begin_primitives.hpp"
#include "messages/header.hpp"
#include "messages/end_primitives.hpp"
#include "messages/clear_screen.hpp"
#include "messages/write_vertex.hpp"
#include "messages/set_perspective.hpp"
#include "messages/swap_buffer.hpp"

#include "qspi.hpp"

#include "mode/modes.hpp"
#include "io/usart_point.hpp"




static auto mode = msgpu::mode::ModesFactory<>()
    .create();

namespace msgpu 
{

static processor::MessageProcessor proc;
static io::UsartPoint usart_io_data; 
static boost::sml::sm<io::UsartPoint> usart_io(usart_io_data);

void frame_update()
{
}

std::size_t fill_scanline(std::span<uint32_t> buffer, std::size_t line)
{
    std::memset(buffer.data(), 0, buffer.size());
    static_cast<void>(line);
    return 0;
}

} // namespace msgpu

void process_frame()
{
}

int main() 
{
    msgpu::initialize_board();
 
    msgpu::initialize_signal_generator();

    msgpu::usart_io.process_event(msgpu::io::init{});
    hal::set_usart_handler([]{
        msgpu::usart_io.process_event(msgpu::io::dma_finished{});
        printf("DMA finished\n");
        if (msgpu::usart_io_data.pop())
        {
            printf("Got data\n");
        }
    });
    
    while (true)
    {
        process_frame();
//        msgpu::sleep_ms(1000);
//        static int i = 0;
//        printf("Working %d\n", ++i);
    }

    msgpu::deinitialize_signal_generator();
} 


