// This file is part of  project.
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

#include <memory>
#include <thread>

#include <cstdio>
#include <iostream>

#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include <SFML/Graphics.hpp>

#include "generator/vga.hpp"

static std::unique_ptr<std::thread> rendering_thread;

bool close_loop = false;
void exit_handler(int sig)
{
    static_cast<void>(sig);
    printf("Received signal\n");
    close_loop = true;
    if (rendering_thread)
        rendering_thread->join();
    exit(0);
}

namespace msgpu
{

static uint16_t resolution_width  = 320;
static uint16_t resolution_height = 240;

static termios old_tio;

void initialize_signal_generator()
{
    termios new_tio;

    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;

    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

static bool enable_dumping = false;

void enable_dump()
{
    // enable_dumping = true;
}

void render_loop()
{
    sf::RenderWindow window(sf::VideoMode(320, 240), "MSGPU");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
    while (window.isOpen())
    {
        if (close_loop)
        {
            return;
        }
        sf::Event ev;
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // msgpu::generator::get_vga().block();

        sf::Image screen;
        sf::Texture screen_texture;
        sf::Sprite screen_sprite;
        screen.create(320, 240, sf::Color::Black);

        for (std::size_t line = 0; line < resolution_height; ++line)
        {
            uint32_t line_buffer_[640];
            msgpu::generator::get_vga().display_line(line, line_buffer_);
            for (int pixel = 0; pixel < resolution_width; ++pixel)
            {
                uint8_t r = (line_buffer_[pixel] >> 5) & 0x07;
                uint8_t g = (line_buffer_[pixel] >> 2) & 0x07;
                uint8_t b = (line_buffer_[pixel] & 0x03);

                r = r * (256 / 8);
                g = g * (256 / 8);
                b = b * (256 / 4);

                // std::cout << "Set color: " << (int)r << ", " << (int)g << ", " << (int)b
                //<< std::endl;
                sf::Color color(r, g, b);
                screen.setPixel(pixel, line, color);
            }
        }
        if (enable_dumping)
        {
            static int i = 0;
            printf("Screenshot: %d.png\n", i);
            std::string filename = std::to_string(i++) + std::string(".png");
            screen.saveToFile(filename);
        }
        screen_texture.loadFromImage(screen);
        screen_sprite.setTexture(screen_texture);
        window.clear();
        window.draw(screen_sprite);
        static std::chrono::time_point<std::chrono::high_resolution_clock> prev;
        window.display();
        // msgpu::generator::get_vga().unblock();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    exit(0);
}

void set_resolution(uint16_t width, uint16_t height)
{
    printf("Setting resolution to: %d %d\n", width, height);
    resolution_width  = width;
    resolution_height = height;
}

void deinitialize_signal_generator()
{
    std::cout << "Joining rendering thread" << std::endl;
    rendering_thread->join();
    std::cout << "Joined" << std::endl;
}

void initialize_application_specific()
{
    printf("================================\n");
    signal(SIGINT, exit_handler);
    const std::string_view dump_frames = std::getenv("DUMP_FRAMES");
    if (dump_frames == "ON" || dump_frames == "1")
    {
        printf("[BOARD/SIM] Enable frames dumping\n");
        enable_dumping = true;
    }
}

void enable_display()
{
    rendering_thread.reset(new std::thread(render_loop));
}

} // namespace msgpu
