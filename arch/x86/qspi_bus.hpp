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
#include <map>
#include <memory> 
#include <optional>

namespace msgpu 
{

class IDevice
{
public: 
    virtual ~IDevice() = default; 
    using DataType = std::span<uint8_t>;
    using ConstDataType = std::span<const uint8_t>;

    virtual void init() = 0;
    virtual void read(const DataType& buf, std::size_t len) = 0;
    virtual void write(const ConstDataType& buf, std::size_t len) = 0;
    virtual void transmit(const ConstDataType& src, const DataType& dest, std::size_t write_len, std::size_t read_len) = 0;
};

class QspiBus 
{
public:
    static QspiBus& get();

    void register_device(int id, std::unique_ptr<IDevice>&& device);

    IDevice* get_device(int id);
    const IDevice* get_device(int id) const;

private:
    QspiBus() = default;

    std::map<int, std::unique_ptr<IDevice>> devices_;
};

} // namespace msgpu
