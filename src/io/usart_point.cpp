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

#include "io/usart_point.hpp"

#include <iostream>

#include "hal_dma.hpp"

namespace msgpu::io 
{

void UsartPoint::prepare_for_header()
{
    std::cerr << "Preparing for header receival" << std::endl;
}

void UsartPoint::prepare_for_token()
{
    std::cerr << "Setup DMA for single byte" << std::endl;
    hal::set_usart_dma_buffer(&token_buffer_, false);
    hal::set_usart_dma_transfer_count(sizeof(token_buffer_), true);
}

} // namespace msgpu::io
