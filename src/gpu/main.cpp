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

//#include <boost/sml.hpp>

//#include "processor/message_processor.hpp"

#include "board.hpp"
#include "hal_dma.hpp"

//#include "messages/change_mode.hpp"
//#include "messages/set_pixel.hpp"
//#include "messages/draw_line.hpp"
//#include "messages/info_req.hpp"
//#include "messages/clear_screen.hpp"
//#include "messages/begin_primitives.hpp"
//#include "messages/end_primitives.hpp"
//#include "messages/write_vertex.hpp"
//#include "messages/write_text.hpp"
//#include "messages/set_perspective.hpp"
//#include "messages/swap_buffer.hpp"
//#include "messages/draw_triangle.hpp"

//#include "messages/begin_primitives.hpp"
//#include "messages/header.hpp"
//#include "messages/end_primitives.hpp"
//#include "messages/clear_screen.hpp"
//#include "messages/write_vertex.hpp"
//#include "messages/set_perspective.hpp"
//#include "messages/swap_buffer.hpp"

#include "qspi.hpp"

#include "mode/modes.hpp"
#include "generator/modes.hpp"
//#include "io/usart_point.hpp"

//#include "mode/3d_graphic_mode.hpp"

//#include "modes/graphic/320x240_256.hpp"

#include <msos/dynamic_linker/dynamic_linker.hpp>
#include <msos/dynamic_linker/environment.hpp>

//#include <eul/error/error_code.hpp>

#include "symbol_codes.h"
#include "arch/i2c.hpp"
#include "arch/pins_config.hpp"
#include "arch/qspi_config.hpp"

//using DualBuffered3DGraphic_320x240_256 = msgpu::mode::DoubleBuffered3DGraphic<msgpu::modes::graphic::Graphic_320x240_256>;

//static auto modes = msgpu::mode::ModesFactory<>()
//    .add_mode<DualBuffered3DGraphic_320x240_256>()
//    .create();

#include "memory/psram.hpp"
#include <ctime>
namespace msgpu 
{


static msos::dl::DynamicLinker dynamic_linker;
//static processor::MessageProcessor proc;
//static io::UsartPoint usart_io_data; 
//static boost::sml::sm<io::UsartPoint> usart_io(usart_io_data);
//static std::size_t get_lot_at(std::size_t address)
//{
//    return dynamic_linker.get_lot_for_module_at(address);
//}

void frame_update()
{
}

std::span<const uint8_t> get_scanline(std::size_t line)
{
//    return ::modes.get_line(line);
    static_cast<void>(line);
    return std::span<const uint8_t>();
}

} // namespace msgpu

void process_frame()
{
}

template <typename MessageType> 
void register_handler()
{
    //msgpu::proc.register_handler<MessageType>(&decltype(modes)::process<MessageType>, &modes);
}

static msos::dl::Environment env {
    msos::dl::SymbolAddress{SymbolCode::libc_printf, &printf},
    msos::dl::SymbolAddress{SymbolCode::libc_puts, &puts}
};
int exec(const std::size_t* module_address)
{
    eul::error::error_code ec;


    const auto* module = msgpu::dynamic_linker.load_module(module_address, msos::dl::LoadingModeCopyText, env, ec);

    if (ec)
    {
        printf("Error during exec: %s\n", ec.message().data());
        return -1; 
    }

    return module->execute();
}
//    static_cast<void>(module_address);
//    return 0;
//}
//
int main() 
{
    msgpu::initialize_board();

    printf("==========================\n");
    printf("=        MSGPU           =\n");
    printf("==========================\n");

    msgpu::Qspi qspi(msgpu::framebuffer_config, 3.0f);
    qspi.init();
    msgpu::memory::QspiPSRAM framebuffer(qspi, true);
    msgpu::I2C i2c(msgpu::i2c_scl, msgpu::i2c_sda);

    char c;
    while (true)
    {
        msgpu::sleep_ms(1000);
        printf("Waiting for command: ");
        scanf("%c", &c);
        printf("\nExecute: %c\n", c);

        constexpr uint8_t set_mode_cmd = 0x02;

        if (c == 'd')
        {
            printf("Write DEMO pattern and trigger RAMDAC to display\n");
            const uint8_t mode = static_cast<uint8_t>(msgpu::modes::Modes::Graphic_320x240_12bit);
            const uint8_t cmd[] = { set_mode_cmd, mode };
            i2c.write(0x2e, cmd);    
        }

        if (c == 'w')
        {
            printf("Write data to memory\n");

            uint8_t cmd_w[] = {0x38, 0x00, 0x00, 0x00};
            uint8_t data[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xaa, 0xff, 0xbb, 0xee, 0xcc, 0x12};
            //framebuffer.write(0x00, data);
            //framebuffer.wait_for_finish();
            qspi.acquire_bus();
            qspi.qspi_command_write(cmd_w, data);
            qspi.release_bus();
            
            uint8_t cmd[] = {0x1, 0x20, 0x0};
            i2c.write(0x2e, cmd);
        }
        printf("Working\n");
    }

    msgpu::deinitialize_signal_generator();
} 


