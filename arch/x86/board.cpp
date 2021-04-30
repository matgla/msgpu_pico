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

namespace msgpu 
{

namespace 
{

static std::unique_ptr<std::thread> rendering_thread;
static termios old_tio; 
static int serial_port_id;

void render_loop()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "MSGPU");
    sf::Image screen;
    sf::Texture screen_texture;
    sf::Sprite screen_sprite;
    
    screen.create(640, 480, sf::Color::Blue);
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
        for (std::size_t line = 0; line < 480; ++line)
        {
            uint32_t line_buffer_[640]; 
            fill_scanline(line_buffer_, line);
            for (int pixel = 0; pixel < 640; ++pixel)
            {
                if (line_buffer_[pixel] != 0)
                screen.setPixel(pixel, line, sf::Color::White);
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

    serial_port_id = open("/dev/pts/2", O_RDWR);
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

void write_bytes(std::span<uint8_t> data)
{
    write(serial_port_id, data.data(), data.size());
}

} // namespace msgpu 

