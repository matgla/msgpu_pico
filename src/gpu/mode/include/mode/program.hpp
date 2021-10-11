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

#include <array>
#include <bitset>

#include <msos/dynamic_linker/loaded_module.hpp>

#include "mode/module.hpp"

namespace msgpu::mode
{

constexpr std::size_t MAX_MODULES_LIST_SIZE = 10;

class Program
{
  public:
    Program() = default;

    bool assign_module(const Module &module)
    {
        if (module.type == ModuleType::VertexShader)
        {
            vertex_shader_ = module.module;
        }
        if (module.type == ModuleType::FragmentShader)
        {
            pixel_shader_ = module.module;
        }
        return true;
    }

    const msos::dl::LoadedModule *vertex_shader_;
    const msos::dl::LoadedModule *pixel_shader_;
};

} // namespace msgpu::mode