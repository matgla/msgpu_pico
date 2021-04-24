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

namespace msgpu 
{

namespace 
{
std::unique_ptr<std::thread> rendering_thread;

void render_loop()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "MSGPU");

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
        window.display();
    }
}

}

void initialize_board()
{

    rendering_thread.reset(new std::thread(&render_loop));
}

void initialize_signal_generator()
{
    
}

void deinitialize_signal_generator()
{
    rendering_thread->join();
}

} // namespace msgpu 

