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

#pragma once

#include <cstdint>
#include <string_view>
#include <variant>
#include <span> 

#include "processor/human_interface.hpp"
#include "processor/machine_interface.hpp"
#include "modes/modes.hpp"

namespace processor
{

class CommandProcessor
{
public:
    using WriteCallback = void(*)(std::span<const uint8_t>);
    CommandProcessor(vga::Mode& mode, WriteCallback callback);

    void change();
    void process_data();
    void process(uint8_t byte);
    void dma_run();
private:
    vga::Mode& mode_;
    WriteCallback write_;
    std::variant<MachineInterface, HumanInterface> interface_;
};

} // namespace processor
