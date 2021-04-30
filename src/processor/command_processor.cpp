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

#include "processor/command_processor.hpp"

namespace processor
{

CommandProcessor::CommandProcessor(vga::Mode& mode, WriteCallback write_callback)
    : mode_(mode)
    , write_(write_callback)
    , interface_(MachineInterface(&mode_, write_callback))
{

}

void CommandProcessor::change()
{
    if (std::holds_alternative<MachineInterface>(interface_))
    {
        interface_ = HumanInterface(mode_, write_);
    }
    else
    {
        interface_ = MachineInterface(&mode_, write_);
    }
}

void CommandProcessor::process(uint8_t data)
{
    std::visit([data](auto&& arg) {
        arg.process(data);
    }, interface_);
}

} // namespace processor
