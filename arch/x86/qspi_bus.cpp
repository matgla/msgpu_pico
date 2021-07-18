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

#include "qspi_bus.hpp"

#include "panic.hpp"

namespace msgpu 
{

QspiBus& QspiBus::get() 
{
    static QspiBus impl;
    return impl;
}

void QspiBus::register_device(int id, std::unique_ptr<IDevice>&& device)
{
    devices_[id] = std::move(device);
}

IDevice* QspiBus::get_device(int id)
{
    return devices_[id].get();
}

const IDevice* QspiBus::get_device(int id) const 
{
    if (!devices_.contains(id))
    {
        panic("Can't find device id: %d, on qspi bus\n", id);
    }
    return devices_.at(id).get();
}

} // namespace msgpu
