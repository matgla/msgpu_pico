// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it is under the terms of the GNU General Public License as published by
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

#include "memory/gpuram.hpp"

namespace msgpu::memory
{
namespace
{
constexpr uint16_t page_size = 1024;
} // namespace
GpuRAM::GpuRAM(QspiPSRAM &memory) : memory_(memory)
{
}

std::size_t GpuRAM::write(std::size_t address, const void *data, std::size_t nbyte)
{
    AddressInfo a = get_address_information(address, nbyte);
    const std::size_t last_page_size = nbyte - a.bytes_to_align - a.pages * page_size;
    std::size_t bytes_written = 0;
    memory_.write(address + bytes_written,
                  QspiPSRAM::ConstDataBuffer(static_cast<const uint8_t *>(data), a.bytes_to_align));
    memory_.wait_for_finish();
    bytes_written += a.bytes_to_align;
    for (uint16_t i = 0; i < a.pages; ++i)
    {
        memory_.write(address + bytes_written,
                      QspiPSRAM::ConstDataBuffer(static_cast<const uint8_t *>(data) + bytes_written,
                                                 page_size));
        bytes_written += page_size;
        memory_.wait_for_finish();
    }

    memory_.write(address + bytes_written,
                  QspiPSRAM::ConstDataBuffer(static_cast<const uint8_t *>(data) + bytes_written,
                                             last_page_size));
    memory_.wait_for_finish();
    return nbyte;
}

std::size_t GpuRAM::read(std::size_t address, void *data, std::size_t nbyte)
{
    AddressInfo a = get_address_information(address, nbyte);
    const std::size_t last_page_size = nbyte - a.bytes_to_align - a.pages * page_size;
    std::size_t bytes_written = 0;
    memory_.read(address + bytes_written,
                 QspiPSRAM::DataBuffer(static_cast<uint8_t *>(data), a.bytes_to_align));
    memory_.wait_for_finish();
    bytes_written += a.bytes_to_align;
    for (uint16_t i = 0; i < a.pages; ++i)
    {
        memory_.write(address + bytes_written,
                      QspiPSRAM::ConstDataBuffer(static_cast<const uint8_t *>(data) + bytes_written,
                                                 page_size));
        bytes_written += page_size;
        memory_.wait_for_finish();
    }

    memory_.write(address + bytes_written,
                  QspiPSRAM::ConstDataBuffer(static_cast<const uint8_t *>(data) + bytes_written,
                                             last_page_size));
    memory_.wait_for_finish();
    return nbyte;
}

GpuRAM::AddressInfo GpuRAM::get_address_information(std::size_t address, std::size_t size)
{
    const uint16_t bytes_to_align = page_size - address % page_size;
    return AddressInfo{
        .bytes_to_align = bytes_to_align,
        .pages = static_cast<uint16_t>((size - bytes_to_align) / page_size),
    };
}

} // namespace msgpu::memory
