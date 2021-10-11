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
    return programs_.allocate();
}

uint8_t Programs::allocate_fragment_shader()
{
    const uint8_t id    = modules_.allocate();
    modules_[id].module = nullptr;
    modules_[id].type   = ModuleType::PixelShader;
    return id;
}

uint8_t Programs::allocate_vertex_shader()
{
    const uint8_t id    = modules_.allocate();
    modules_[id].module = nullptr;
    modules_[id].type   = ModuleType::VertexShader;
    return id;
}

bool Programs::add_shader(uint8_t module_id, const msos::dl::LoadedModule *module)
{
    if (!modules_.test(module_id))
    {
        return false;
    }

    modules_[module_id].module = module;
    return true;
}

bool Programs::assign_module(uint8_t program_id, uint8_t module_id)
{
    if (!programs_.test(program_id) || !modules_.test(module_id))
    {
        return false;
    }

    printf("Assigned %d, to %d\n", module_id, program_id);
    programs_[program_id].assign_module(modules_[module_id]);
    return true;
}

const Program *Programs::get(uint8_t program_id) const
{
    if (!programs_.test(program_id))
    {
        return nullptr;
    }
    return &programs_[program_id];
}

} // namespace msgpu::mode
