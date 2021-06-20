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
//#include "io/usart_point.hpp"

//#include "mode/3d_graphic_mode.hpp"

//#include "modes/graphic/320x240_256.hpp"

#include <msos/dynamic_linker/dynamic_linker.hpp>
#include <msos/dynamic_linker/environment.hpp>

//#include <eul/error/error_code.hpp>

#include "symbol_codes.h"

#include "pico/stdlib.h"

//using DualBuffered3DGraphic_320x240_256 = msgpu::mode::DoubleBuffered3DGraphic<msgpu::modes::graphic::Graphic_320x240_256>;

//static auto modes = msgpu::mode::ModesFactory<>()
//    .add_mode<DualBuffered3DGraphic_320x240_256>()
//    .create();

#include "memory/psram.hpp"

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

int main() 
{
    msgpu::initialize_board();
 
    //msgpu::initialize_signal_generator();

    printf("Hello from msgpu\n");
//    msgpu::usart_io.process_event(msgpu::io::init{});

//    register_handler<BeginPrimitives>();
//    register_handler<EndPrimitives>();
//    register_handler<WriteVertex>();
//    register_handler<ClearScreen>();
//    register_handler<SwapBuffer>();

//    hal::set_usart_handler([]{
//        msgpu::usart_io.process_event(msgpu::io::dma_finished{});
//     });

//    modes.switch_to<DualBuffered3DGraphic_320x240_256>();
   
  //  printf("Sizeof: %ld\n", sizeof(DualBuffered3DGraphic_320x240_256::FrameBufferType::BufferType));
  //  printf("Sizeof mode: %ld\n", sizeof(DualBuffered3DGraphic_320x240_256));
  //  printf("Sizeof base: %ld\n", sizeof(DualBuffered3DGraphic_320x240_256::Base));
    //exec(reinterpret_cast<const std::size_t*>(
    //
    printf("Testing QSPI module\n");
  //      Qspi qspi;
  //  qspi.init();
  //  qspi.switch_to(Qspi::Mode::SPI);

  //  qspi.chip_select(Qspi::Device::Ram, false);

  //  sleep_us(200);
  //  uint8_t reset_en[] = {0x66};
   
  //  qspi.chip_select(Qspi::Device::Ram, true);
  //  qspi.spi_write8(reset_en);
  //  qspi.chip_select(Qspi::Device::Ram, false);
  //  uint8_t reset_cmd[] = {0x99};
  //  qspi.chip_select(Qspi::Device::Ram, true);
  //  qspi.spi_write8(reset_cmd);
  //  qspi.chip_select(Qspi::Device::Ram, false);
  //  sleep_us(100);

  //  qspi.chip_select(Qspi::Device::Ram, true);

  //  uint8_t read_eid[] = {0x9f, 0x00, 0x00, 0x00};
  //  uint8_t eid[8] = {};
  //  qspi.spi_write8(read_eid);
  //  qspi.spi_read8(eid);
  //  qspi.chip_select(Qspi::Device::Ram, false);

  //  printf("EID: ");
  //  for (auto b : eid)
  //  {
  //      printf("0x%x, ", b);
  //  }
  //  printf("\n");
 //   msgpu::memory::QspiPSRAM framebuffer(Qspi::Device::Ram);
//    if (!framebuffer.init())
//    {
//        printf("QSPI initialization error\n");
//        while (true) {}
//    }
    Qspi qspi(Qspi::Device::framebuffer, 125.f);//1.95f);
    qspi.init();

    msgpu::memory::QspiPSRAM framebuffer(qspi);
    if (!framebuffer.init())
    {
        printf("QSPI intialization error\n");
    }
 
    uint8_t buf[255];
        const uint8_t buffer[] = {0xff, 0x4, 0x2, 0x3, 0x11, 0x22, 0x33, 0xff, 0x12, 0x23, 0x1, 0x2, 0x3};
        framebuffer.write(0x10, buffer);
 
    while (true)
    {
        static int i = 0;
       
        uint8_t readed[sizeof(buffer)] = {};
        framebuffer.read(0x10, readed);

        printf("Readed: { ");
        for (auto b : readed)
        {
            printf("0x%x, ", b);
        }
        printf(" }\n");

        uint8_t read_command[] = {0x0b, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        uint8_t read_data[sizeof(read_command)] = {};
        framebuffer.exit_qpi_mode();
        msgpu::sleep_us(5);
        qspi.spi_transmit(read_command, read_data);
        msgpu::sleep_us(5);
        framebuffer.enter_qpi_mode();
        printf("SPI readed: { ");
        for (auto b : read_data)
        {
            printf("0x%x, ", b);
        }
        printf(" }\n");

        static int n = 0;
        if (n++ > 5) while (true);
        

//        static uint8_t byte = 1;
//        uint8_t command[] = {0x02, 0x00, 0x00, 0x00, 0xaa, 0xab, 0xfa, 0xce, byte++};

//        qspi.chip_select(Qspi::Device::Ram, true);
//        qspi.spi_write8(command);
//        qspi.chip_select(Qspi::Device::Ram, false);
//        uint8_t tmp[9] = {0x03, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff};
//        uint8_t readed[9] = {};
//        sleep_us(5);
//        qspi.chip_select(Qspi::Device::Ram, true);
//        qspi.read8_write8_blocking(readed, tmp);
//        qspi.chip_select(Qspi::Device::Ram, false);

//        printf("Readed: ");
//        for (const auto b : readed)
//        {
//            printf("0x%x, ", b);
//        }
//        printf ("\n");


 //       auto message = msgpu::usart_io_data.pop();
 //       if (message)
 //       {
 //           msgpu::proc.process_message(*message);
 //       }

    }

    msgpu::deinitialize_signal_generator();
} 


