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
#include <span>

#include <msos/dynamic_linker/loaded_module.hpp>

#include "mode/module.hpp"
#include "mode/program.hpp"

namespace msgpu::mode
{

namespace
{

constexpr std::size_t MAX_PROGRAM_LIST_SIZE = 10;

} // namespace
class Programs
{
  public:
    Programs();
    uint8_t allocate_program();
    uint8_t allocate_module();

    bool add_fragment_shader(uint8_t module_id, const msos::dl::LoadedModule *module);
    bool add_vertex_shader(uint8_t module_id, const msos::dl::LoadedModule *module);
    bool assign_module(uint8_t program_id, uint8_t module_id);

    const Program *get(uint8_t program_id) const;

  private:
    std::array<Module, MAX_MODULES_LIST_SIZE> modules_;
    std::bitset<MAX_MODULES_LIST_SIZE> modules_map_;

    std::array<Program, MAX_PROGRAM_LIST_SIZE> programs_;
    std::bitset<MAX_PROGRAM_LIST_SIZE> programs_map_;

    uint8_t used_program_;
};

} // namespace msgpu::mode
