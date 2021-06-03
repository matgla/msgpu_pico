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

#include "mode/3d_graphic_mode.hpp"

#include "modes/graphic/320x240_256.hpp"

using DualBuffered3DGraphic_320x240_256 = msgpu::mode::DoubleBuffered3DGraphic<msgpu::modes::graphic::Graphic_320x240_256>;

static auto modes = msgpu::mode::ModesFactory<>()
    .add_mode<DualBuffered3DGraphic_320x240_256>()
    .create();

namespace msgpu 
{

std::size_t fill_scanline(std::span<uint32_t> buffer, std::size_t line)
{
    modes.fill_line(buffer, line);
}

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

template <typename MessageType> 
void register_handler()
{
    msgpu::proc.register_handler<MessageType>(&decltype(modes)::process<MessageType>, &modes);
}

int main() 
{
    msgpu::initialize_board();
 
    msgpu::initialize_signal_generator();

    msgpu::usart_io.process_event(msgpu::io::init{});

    register_handler<BeginPrimitives>();
    register_handler<EndPrimitives>();
    register_handler<WriteVertex>();
    register_handler<ClearScreen>();

    hal::set_usart_handler([]{
        msgpu::usart_io.process_event(msgpu::io::dma_finished{});
     });

    modes.switch_to<DualBuffered3DGraphic_320x240_256>();
    
    while (true)
    {
        process_frame();
        auto message = msgpu::usart_io_data.pop();
        if (message)
        {
            msgpu::proc.process_message(*message);
        }
    }

    msgpu::deinitialize_signal_generator();
} 


