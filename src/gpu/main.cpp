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
#include "arch/qspi_config.hpp"

#include "mode/modes.hpp"
//#include "io/usart_point.hpp"

//#include "mode/3d_graphic_mode.hpp"

//#include "modes/graphic/320x240_256.hpp"

#include <msos/dynamic_linker/dynamic_linker.hpp>
#include <msos/dynamic_linker/environment.hpp>

//#include <eul/error/error_code.hpp>

#include "symbol_codes.h"

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
constexpr std::size_t benchmark_rows = 2;
static uint8_t test_data[benchmark_rows][45240];
static uint8_t read_data[benchmark_rows][45240];
void generate_data()
{
    srand(msgpu::get_us());
    
    for (auto& d : test_data)
    {
        for (auto& b : d)
        {
            b = rand() % 0xff; 
        }
    }
}

void benchmark(msgpu::memory::QspiPSRAM& memory)
{
    printf("=====Begin test=====\n");
    uint32_t start_time = msgpu::get_us();
    uint32_t address = 0; 
    uint32_t setup_time = 0;
    uint32_t finish_time = 0;
    for (const auto& line : test_data)
    {
        memory.write(address, line);
        address += sizeof(test_data[0]);
    }
    uint32_t write_end_time = msgpu::get_us();
    printf("Start time: %d\n", start_time);
    printf("Write finished: %d\n", write_end_time);
    printf("Took %d\n", write_end_time - start_time);
    printf("Setup took %d\n", finish_time - setup_time);
    printf("Speed: %f MB/s\n", static_cast<float>(sizeof(test_data)) / static_cast<float>(write_end_time - start_time));
    uint32_t read_start_time = msgpu::get_us();
    address = 0;
    for (auto& line : read_data)
    {
        memory.read(address, line);
        address += sizeof(test_data[0]);
    }
    uint32_t read_end_time = msgpu::get_us();
    printf("Reading start: %d\n", read_start_time);
    printf("Reading end: %d\n", read_end_time);
    printf("Took %d\n", read_end_time - read_start_time);
    printf("Speed: %f MB/s\n", static_cast<float>(sizeof(test_data)) / static_cast<float>(read_end_time - read_start_time));

    printf("====Verification started====\n");
    int success = 0;
    int failure = 0;
    for (int y = 0; y < benchmark_rows; ++y)
    {
        if (y == 0)
        {
            printf("Data: ");
        }
        for (int x = 0; x < sizeof(test_data[0]); ++x)
        {
            if (y == 0 && x < 32) 
            {
                printf("0x%x (0x%x), ", read_data[y][x], test_data[y][x]);
                if (x % 16 == 15) 
                {
                    printf("\n");
                }
            }
            if (test_data[y][x] != read_data[y][x])
            {
                ++failure;
                if (y == 0 && x < 32)
                {
                }else {
                break;
                }
            }
        }
        if (y == 0)
        {
            printf("\n");
        }
        ++success;
    }
    printf("Failed: %d/%d\n", failure, success);
}

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
    Qspi qspi(msgpu::framebuffer_config, 3.0f);//1.95f);
    qspi.init();

    msgpu::memory::QspiPSRAM framebuffer(qspi);
    if (!framebuffer.init())
    {
        printf("QSPI intialization error\n");
        while (true) {}
    }

    generate_data();
    benchmark(framebuffer);


    while (true)
    {

    }

    msgpu::deinitialize_signal_generator();
} 


