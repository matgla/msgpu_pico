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

#include <cstdio>
#define layout(arg) extern __attribute__((alias(argument_##arg)))

int main()
{
    vec3 aPos = *reinterpret_cast<vec3 *>(*in_argument[0]);
    printf("VertexShaderCalled: %f %f %f\n", aPos.x, aPos.y, aPos.z);
    gl_Position = vec4(aPos, 1.0);
}