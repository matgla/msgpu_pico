/*
 *   Copyright (c) 2021 Mateusz Stadnik

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>
#include <bitset>
#include <cstring>
#include <string_view>

#include <msos/dynamic_linker/loaded_module.hpp>

#include "mode/indexed_buffer.hpp"
#include "mode/module.hpp"

namespace msgpu::mode
{

constexpr std::size_t MAX_MODULES_LIST_SIZE = 10;

class Program
{
  public:
    Program();

    bool assign_module(const Module &module);

    uint8_t get_named_parameter_id(std::string_view name);
    std::string_view get_parameter_name(uint8_t id) const;

    const msos::dl::LoadedModule *pixel_shader() const;
    const msos::dl::LoadedModule *vertex_shader() const;

    void delete_parameter_by_id(uint8_t id);
    void delete_parameter_by_name(std::string_view name);

  protected:
    struct NamedParameter
    {
        char name[20];
    };

    const msos::dl::LoadedModule *vertex_shader_;
    const msos::dl::LoadedModule *pixel_shader_;
    IndexedBuffer<NamedParameter, 5, uint8_t> named_parameters_;
};

} // namespace msgpu::mode