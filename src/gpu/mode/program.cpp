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

#include "mode/program.hpp"

namespace msgpu::mode
{

Program::Program()
    : vertex_shader_(nullptr)
    , pixel_shader_(nullptr)
    , named_parameters_{}
{
}

bool Program::assign_module(const Module &module)
{
    if (module.type == ModuleType::VertexShader)
    {
        vertex_shader_ = module.module;
    }
    if (module.type == ModuleType::PixelShader)
    {
        pixel_shader_ = module.module;
    }
    return true;
}

uint8_t Program::get_named_parameter_id(std::string_view name)
{
    for (uint8_t i = 0; i < named_parameters_.size(); ++i)
    {
        if (named_parameters_.test(i))
        {
            if (std::string_view(named_parameters_[i].name) == name)
            {
                return i;
            }
        }
    }

    const uint8_t id = named_parameters_.allocate();
    std::memcpy(named_parameters_[id].name, name.data(), name.size());
    return id;
}

std::string_view Program::get_parameter_name(uint8_t id) const
{
    if (!named_parameters_.test(id))
    {
        return "";
    }
    return named_parameters_[id].name;
}

const msos::dl::LoadedModule *Program::pixel_shader() const
{
    return pixel_shader_;
}

const msos::dl::LoadedModule *Program::vertex_shader() const
{
    return vertex_shader_;
}

void Program::delete_parameter_by_id(uint8_t id)
{
    if (named_parameters_.test(id))
    {
        named_parameters_.deallocate(id);
    }
}

void Program::delete_parameter_by_name(std::string_view name)
{
    for (uint8_t i = 0; i < named_parameters_.size(); ++i)
    {
        if (named_parameters_.test(i))
        {
            if (std::string_view(named_parameters_[i].name) == name)
            {
                named_parameters_.deallocate(i);
                return;
            }
        }
    }
}

} // namespace msgpu::mode