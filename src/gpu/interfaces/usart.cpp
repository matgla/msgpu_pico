// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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

#include "interfaces/usart.hpp"

#include <eul/utils/string.hpp>


void Usart::initialize()
{
    
}

uint8_t buffer[50];

void Usart::write(const std::string_view& msg)
{
}

void Usart::write(char c)
{
}

void Usart::write(int n)
{
    char buf[100];
    eul::utils::itoa(n, buf, 10);
}

void Usart::write_hex(int n)
{
    char buf[100];
    eul::utils::itoa(n, buf, 16);
}


std::string_view Usart::read()
{
    return "";
}
