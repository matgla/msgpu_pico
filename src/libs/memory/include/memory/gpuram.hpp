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

#pragma once

#include <cstdint>
#include <cstdlib>

#include "memory/psram.hpp"

namespace msgpu::memory
{

/// @brief Manages PSRAM module for GPU memory

class GpuRAM
{
  public:
    /// @brief Constructs GpuRAM object
    ///
    /// @param[in] memory - memory handler object
    GpuRAM(QspiPSRAM &memory);

    /// @brief Write data to memory with respect to page crossing
    ///
    /// @param[in] address - address to write to
    /// @param[in] data - data to write
    /// @param[in] nbyte - size of data to write
    ///
    /// @returns size of written bytes

    std::size_t write(std::size_t address, const void *data, std::size_t nbyte);

    /// @brief Read data from memory with respect to page crossing
    ///
    /// @param[in] address - address to read from
    /// @param[out] data - buffer to store data
    /// @param[in] nbyte - size of data to read
    ///
    /// @returns size of readed data
    std::size_t read(std::size_t address, void *data, std::size_t nbyte);

  private:
    struct AddressInfo
    {
        uint16_t bytes_to_align;
        uint16_t pages;
    };

    /// @brief a
    ///
    /// @param[in] address - base from which calculate pages information
    /// @param[in] size - size of data to be transferred
    ///
    /// @returns AddressInfo
    static AddressInfo get_address_information(std::size_t address, std::size_t size);

    QspiPSRAM &memory_;
};

} // namespace msgpu::memory
