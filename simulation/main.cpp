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

#include <iostream> 

#include <SFML/Graphics.hpp>

int main(int argc, char* argv[]) 
{
    std::cerr << "========================================" << std::endl;
    std::cerr << "=          MSGPU SIMULATION            =" << std::endl;
    std::cerr << "========================================" << std::endl;

    sf::Window app(sf::VideoMode(800, 600), "MSGPU sim");

    while (app.isOpen())
    {
        sf::Event event;
        while (app.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                app.close();
            }
            app.display();
        }
    }
} 
