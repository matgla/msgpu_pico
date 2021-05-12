// This file is part of MSGPU project.
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

#include <cstdio> 
#include <cstring> 

#include <unistd.h>

#include "modes/modes.hpp"

#include "processor/command_processor.hpp"
#include "processor/human_interface.hpp"

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

void frame_update()
{
    mode.render();
}

} // namespace msgpu


struct Vertex
{
    float x;
    float y;
    float z;
};

struct Triangle 
{
    Vertex v[3];
};

struct Mesh 
{
    Triangle triangles[12]; 
};

const Mesh mesh {
    .triangles = {
        {{ {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f} }},
        {{ {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }},
        {{ {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f} }},
        {{ {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f} }},
        {{ {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} }},
        {{ {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} }},
        {{ {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} }},
        {{ {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }},
        {{ {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} }},
        {{ {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f} }},
        {{ {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} }},
        {{ {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }},
    }
};

template <typename T, typename C>
void write_msg(T& msg, C& c)
{
    Header header;
    header.id = T::id;
    header.size = sizeof(T);

    uint8_t* bytes = reinterpret_cast<uint8_t*>(&header);
    for (std::size_t i = 0; i < sizeof(Header); ++i)
    {
        c.process(bytes[i]);
    }

    bytes = reinterpret_cast<uint8_t*>(&msg);
    for (std::size_t i = 0; i < header.size; ++i)
    {

        c.process(bytes[i]);
    }
}

int main() 
{
    msgpu::initialize_board();
 
    msgpu::initialize_signal_generator();
    
    static processor::CommandProcessor processor(mode, &msgpu::write_bytes);

//    SetPerspective p;
//    p.aspect = 1.0;
//    p.view_angle = 90;
//    p.z_far = 1000.0;
//    p.z_near = 1.0;

//    write_msg(p, processor);
    uint8_t d[] = {'h', 'e', 'j'};
    msgpu::write_bytes(d);

    printf("============================\n\n\n");
    uint8_t buf_in[8];
    Qspi::init();
    Qspi::switch_to(Qspi::Mode::QSPI_write);
    //Qspi::switch_to(Qspi::Mode::SPI);
    printf("QSPI test\n");
    uint8_t buf_out[8] = {0xaa, 0xbb, 0xcd, 0xff, 0x5, 0x6, 0x7, 0x8 };
    while (true)
    {
        //Qspi::switch_to(Qspi::Mode::SPI);
        //Qspi::read8_write8_blocking(buf_in, buf_out);
        Qspi::qspi_write8(buf_out);
        
        //buf_out[0] = 0xa;
       // buf_out[1] = 0xb;
        printf("Data: ");

        for (uint8_t b : buf_in)
        {
            printf("%d, ", b);
        }
        printf("\n");
        
     //   printf("\nSending write/read not in parallel\n");
 
    //    Qspi::spi_write8(buf_out);

//        Qspi::spi_read8(buf_in);
//        printf("Data: ");
//        for (uint8_t b : buf_in)
//        {
//            printf("%d, ", b);
//        }
//        printf("\n");

//        printf ("Switch to QSPI\n");
//        Qspi::switch_to(Qspi::Mode::QSPI_write);

//        buf_out[2] = 0xde;
//        buf_out[3] = 0xbe;

//        printf("Write QSPI\n");
//        Qspi::qspi_write8(buf_out);
//        printf("Read QSPI\n");
//        Qspi::qspi_read8(buf_in);

//        printf("Data qspi: ");
//        for (uint8_t b : buf_in)
//        {
//            printf("%d, ", b);
//        }
//        printf("\n");

 
        msgpu::sleep_ms(1000);
//        uint8_t byte = msgpu::read_byte();
//        processor.process(byte);
//        uint32_t start_ms = msgpu::get_millis();
//        ClearScreen clr{};
//        write_msg(clr, processor);
//        BeginPrimitives b{};
//        b.type = PrimitiveType::triangle;
//        write_msg(b, processor);


//        for (int i = 0; i < 12; ++i)
//        {
//            for (int j = 0; j < 3; ++j)
//            {
//                WriteVertex v{}; 
//                v.x = mesh.triangles[i].v[j].x;
//                v.y = mesh.triangles[i].v[j].y;
//                v.z = mesh.triangles[i].v[j].z;

//                write_msg(v, processor);
//            }
//        }
        
//        EndPrimitives e{};
//        write_msg(e, processor);
//        SwapBuffer s{};
//        write_msg(s, processor);
//        uint32_t end_ms = msgpu::get_millis();

//        uint32_t diff = end_ms - start_ms; 
//        if (diff < 20)
//        msgpu::sleep_ms(20 - diff);
    }

    msgpu::deinitialize_signal_generator();
} 


