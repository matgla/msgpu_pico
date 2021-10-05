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

#include "mode/programs.hpp"

namespace msgpu::mode
{

Programs::Programs()
    : used_program_(MAX_PROGRAM_LIST_SIZE)
{
}

uint8_t Programs::allocate_program()
{
    for (uint8_t i = 0; i < programs_map_.size(); ++i)
    {
        if (programs_map_.test(i) == 0)
        {
            programs_map_[i] = 1;
            programs_[i]     = Program();
            return i;
        }
    }
    return -1;
}

uint8_t Programs::allocate_module()
{
    for (uint8_t i = 0; i < modules_map_.size(); ++i)
    {
        if (modules_map_.test(i) == 0)
        {
            modules_map_[i] = 1;
            return i;
        }
    }
    return -1;
}

bool Programs::add_vertex_shader(uint8_t module_id, const msos::dl::LoadedModule *module)
{
    if (!modules_map_.test(module_id))
    {
        return false;
    }

    modules_[module_id] = {
        .type   = ModuleType::VertexShader,
        .module = module,
    };
    return true;
}

bool Programs::add_fragment_shader(uint8_t module_id, const msos::dl::LoadedModule *module)
{
    if (!modules_map_.test(module_id))
    {
        return false;
    }

    modules_[module_id] = {
        .type   = ModuleType::FragmentShader,
        .module = module,
    };
    return true;
}

bool Programs::assign_module(uint8_t program_id, uint8_t module_id)
{
    if (!programs_map_.test(program_id) || !modules_map_.test(module_id))
    {
        return false;
    }

    programs_[program_id].assign_module(modules_[module_id]);
    return true;
}

const Program *Programs::get(uint8_t program_id) const
{
    if (!programs_map_.test(program_id))
    {
        return nullptr;
    }
    return &programs_[program_id];
}

} // namespace msgpu::mode
