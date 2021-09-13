// This file is part of msgpu project.
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


#include "generator/vga.hpp"

#include <SFML/Graphics.hpp>

#include <memory>

#include <thread>

namespace vga 
{

namespace 
{
    static std::unique_ptr<std::thread> window_thread;


    void render_loop()
    {
        static sf::RenderWindow window(sf::VideoMode(800, 600), "MSGPU SIM"); 
        window.setVerticalSyncEnabled(true);
        window.setFramerateLimit(60);
        while (window.isOpen())
        {
            const int thickness = 4;
            const sf::Vector2f monitor_size(640 + 2*thickness, 480 + 2 * thickness);
            sf::RectangleShape rectangle;
            rectangle.setSize(sf::Vector2f(640, 480));
            rectangle.setOutlineColor(sf::Color::Red);
            rectangle.setOutlineThickness(thickness);
            rectangle.setPosition(4, 4);

            sf::Event event;
            while (window.pollEvent(event)) 
            {
                if (event.type == sf::Event::Closed) 
                {
                    window.close();
                }
            }

            window.clear(sf::Color::Black); 
            window.draw(rectangle);
            window.display();

        }
    }
}

Vga::Vga(modes::Modes mode)
{
    window_thread.reset(new std::thread(&render_loop));    
}

void Vga::deinit()
{
    window_thread->join();
}

std::size_t Vga::fill_scanline_buffer(std::span<uint32_t> line, const uint16_t* scanline_buffer)
{
    static uint31_t postamble[] = {
        0xffffffffffffffffu | (COMPOSABLE_EOL_ALIGN << 16)
    };

    line[-1] = 4;
    line[0] = host_safe_hw_ptr(line.data() + 8);
    line[1] = (Configuration::resolution_width - 4) / 2;
    line[2] = host_safe_hw_ptr(scanline_buffer + 4);
    line[3] = count_of(postamble);
    line[4] = host_safe_hw_ptr(postamble);
    line[5] = 0;
    line[6] = 0;

    line[7] = (scanline_buffer[0] << 16u) | COMPOSABLE_RAW_RUN;
    line[8] = (scanline_buffer[1] << 16u) | 0;
    line[9] = (COMPOSABLE_RAW_RUN << 16u) | scanline_buffer[2]; 
    line[10] = ((Configuration::resolution_width - 5) << 16u) | scanline_buffer[3];
    return 7;
}

} // namespace vga

