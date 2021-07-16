// This file is part of msgput project.
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

#include <SFML/Graphics.hpp>

#include <thread>
#include <memory>

#include <unistd.h>
#include <termios.h> 
#include <fcntl.h>

#include "generator/vga.hpp"

namespace msgpu 
{
namespace 
{
static int serial_port_id;
static std::unique_ptr<std::thread> rendering_thread;
static termios old_tio; 

static uint16_t resolution_width = 320;
static uint16_t resolution_height = 240;

void render_loop()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "MSGPU");
    sf::Image screen;
    sf::Texture screen_texture;
    sf::Sprite screen_sprite;
    
    screen.create(640, 480, sf::Color::Black);
    screen_texture.loadFromImage(screen);

    screen_sprite.setTexture(screen_texture);
    while (window.isOpen())
    {
        sf::Event ev; 
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        window.clear();
        msgpu::frame_update();
        for (std::size_t line = 0; line < resolution_height ; ++line)
        {
            uint32_t line_buffer_[640]; 
            auto scanline = get_scanline(line);
            get_vga().display_line(line_buffer_, scanline);
            for (int pixel = 0; pixel < resolution_width; ++pixel)
            {
                uint8_t r = (line_buffer_[pixel] >> 8) & 0xf;
                uint8_t g = (line_buffer_[pixel] >> 4) & 0xf;
                uint8_t b = (line_buffer_[pixel] & 0xf);
                
                r = r * (256/16 + 1);
                g = g * (256/16 + 1);
                b = b * (256/16 + 1);

                sf::Color color(r, g, b);
                screen.setPixel(pixel, line, color);
            }
        }
        screen_texture.loadFromImage(screen);
        screen_sprite.setTexture(screen_texture);
        window.draw(screen_sprite);
        window.display();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    exit(0);
}

}

void initialize_board()
{
    rendering_thread.reset(new std::thread(&render_loop));

    serial_port_id = open("/tmp/msgpu_virtual_serial_0", O_RDWR);
}


void initialize_signal_generator()
{
    termios new_tio;

    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;

    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void deinitialize_signal_generator()
{
    rendering_thread->join();
}

uint8_t read_byte() 
{
    uint8_t byte; 
    read(serial_port_id, &byte, sizeof(byte));
    return byte;
}

void write_bytes(std::span<const uint8_t> data)
{
    write(serial_port_id, data.data(), data.size());
}

void set_resolution(uint16_t width, uint16_t height)
{
    printf("Setting resolution to: %d %d\n", width, height);
    resolution_width = width;
    resolution_height = height;
}

void sleep_ms(uint32_t time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

uint32_t get_millis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64_t get_us() 
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void sleep_us(uint32_t time)
{
    std::this_thread::sleep_for(std::chrono::microseconds(time));
}

} // namespace msgpu 

