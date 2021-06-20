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

#pragma once 

#include <cstdint>
#include <span>

#include "qspi.hpp"

namespace msgpu::memory 
{

class QspiPSRAM
{
public:
    QspiPSRAM(Qspi& qspi);

    bool init();
    bool reset();

    using DataBuffer = std::span<uint8_t>;
    using ConstDataBuffer = std::span<const uint8_t>;
    std::size_t write(const std::size_t address, const ConstDataBuffer data);
    std::size_t read(const std::size_t address, DataBuffer data);
//private:
    bool perform_post();
    void enter_qpi_mode();
    void exit_qpi_mode();
   
    Qspi& qspi_;
};

} // namespace msgpu::memory

