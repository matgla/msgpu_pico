// This file is part of MSGPU project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include <cstdio>
#include <cstring>

#include <thread>
#include <unistd.h>

#include <boost/sml.hpp>

#include "io/usart_point.hpp"
#include "processor/message_processor.hpp"

#include "board.hpp"
#include "hal_dma.hpp"

#include "messages/begin_primitives.hpp"
#include "messages/bind.hpp"
#include "messages/change_mode.hpp"
#include "messages/clear_screen.hpp"
#include "messages/draw_arrays.hpp"
#include "messages/draw_line.hpp"
#include "messages/draw_triangle.hpp"
#include "messages/end_primitives.hpp"
#include "messages/generate_names.hpp"
#include "messages/info_req.hpp"
#include "messages/set_perspective.hpp"
#include "messages/set_pixel.hpp"
#include "messages/swap_buffer.hpp"
#include "messages/write_buffer_data.hpp"
#include "messages/write_text.hpp"
#include "messages/write_vertex.hpp"
//#include "messages/begin_primitives.hpp"
//#include "messages/header.hpp"
//#include "messages/end_primitives.hpp"
//#include "messages/clear_screen.hpp"
//#include "messages/write_vertex.hpp"
//#include "messages/set_perspective.hpp"
//#include "messages/swap_buffer.hpp"

#include "qspi.hpp"

#include "generator/modes.hpp"
#include "memory/gpuram.hpp"
#include "memory/vram.hpp"
#include "mode/modes.hpp"

#include "hal_dma.hpp"
//#include "io/usart_point.hpp"

#include "mode/3d_graphic_mode.hpp"

#include "modes/graphic/320x240_256.hpp"

#include <msos/dynamic_linker/dynamic_linker.hpp>
#include <msos/dynamic_linker/environment.hpp>

//#include <eul/error/error_code.hpp>

#include "arch/i2c.hpp"
#include "arch/pins_config.hpp"
#include "arch/qspi_config.hpp"
#include "symbol_codes.h"

#include <condition_variable>

#include "memory/psram.hpp"
#include <ctime>

#include "log/log.hpp"

using DualBuffered3DGraphic_320x240_256 =
    msgpu::mode::DoubleBuffered3DGraphic<msgpu::modes::graphic::Graphic_320x240_256, msgpu::I2C>;

static auto modes =
    msgpu::mode::ModesFactory<>().add_mode<DualBuffered3DGraphic_320x240_256>().create();
namespace msgpu
{

static msos::dl::DynamicLinker dynamic_linker;
// static processor::MessageProcessor proc;
// static io::UsartPoint usart_io_data;
// static boost::sml::sm<io::UsartPoint> usart_io(usart_io_data);
// static std::size_t get_lot_at(std::size_t address)
//{
//    return dynamic_linker.get_lot_for_module_at(address);
//}

} // namespace msgpu

template <typename MessageType>
void register_handler(auto &proc)
{
    proc.template register_handler<MessageType>(&decltype(modes)::process<MessageType>, &modes);
}

static msos::dl::Environment env{msos::dl::SymbolAddress{SymbolCode::libc_printf, &printf},
                                 msos::dl::SymbolAddress{SymbolCode::libc_puts, &puts}};
int exec(const std::size_t *module_address)
{
    eul::error::error_code ec;

    const auto *module =
        msgpu::dynamic_linker.load_module(module_address, msos::dl::LoadingModeCopyText, env, ec);

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
//

void register_messages(auto &proc)
{
    register_handler<ChangeMode>(proc);
    register_handler<SetPixel>(proc);
    register_handler<DrawLine>(proc);
    register_handler<InfoReq>(proc);
    register_handler<ClearScreen>(proc);
    register_handler<BeginPrimitives>(proc);
    register_handler<EndPrimitives>(proc);
    register_handler<WriteVertex>(proc);
    register_handler<WriteText>(proc);
    register_handler<SetPerspective>(proc);
    register_handler<SwapBuffer>(proc);
    register_handler<DrawTriangle>(proc);
    register_handler<GenerateNamesRequest>(proc);
    register_handler<WriteBufferData>(proc);
    register_handler<BindObject>(proc);
    register_handler<PrepareForData>(proc);
    register_handler<DrawArrays>(proc);
};

struct ControlUsart
{
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> trigger;
};

int main()
{
    msgpu::initialize_board();

    printf("==========================\n");
    printf("=        MSGPU           =\n");
    printf("==========================\n");

    msgpu::Qspi qspi(msgpu::framebuffer_config, 3.0f);
    msgpu::Qspi qspi_ram(msgpu::gpuram_config, 3.0f);

    qspi.init();
    qspi_ram.init();

    msgpu::memory::QspiPSRAM qspi_memory(qspi, true);
    msgpu::memory::VideoRam framebuffer(qspi_memory);

    msgpu::memory::QspiPSRAM qspi_gpuram(qspi_ram, true);
    msgpu::memory::GpuRAM gpuram(qspi_gpuram);
    msgpu::I2C i2c(msgpu::i2c_scl, msgpu::i2c_sda);
    msgpu::io::UsartPoint usart_io_data;

    boost::sml::sm<msgpu::io::UsartPoint> usart_io(usart_io_data);

    ControlUsart c;
    hal::set_usart_handler([&c]() {
        c.trigger = true;
        c.cv.notify_all();
    });

    modes.switch_to<DualBuffered3DGraphic_320x240_256>(framebuffer, gpuram, i2c, usart_io_data);
    msgpu::processor::MessageProcessor proc;
    register_messages(proc);

    usart_io.process_event(msgpu::io::init{});

    while (true)
    {
        {
            std::unique_lock lk(c.mutex);
            if (!c.trigger)
            {
                // std::this_thread::sleep_for(std::chrono::microseconds(10));
                if (!c.cv.wait_for(lk, std::chrono::microseconds(100),
                                   [&c] { return c.trigger.load(); }))
                    continue;
            }
            c.trigger = false;
        }

        usart_io.process_event(msgpu::io::dma_finished{});
        auto message = usart_io_data.pop();
        if (message)
        {
            proc.process_message(*message);
        }
    }
}

