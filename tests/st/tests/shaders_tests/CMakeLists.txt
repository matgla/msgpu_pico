# This file is part of MSGPU project. 
# Copyright (C) 2021 Mateusz Stadnik
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

add_library(shader_stub SHARED)

target_sources(shader_stub 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/shader_stub.cpp
)


add_library(vertex_shader_stub SHARED)

target_sources(vertex_shader_stub 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/vertex_shader_stub.cpp
)

add_library(pixel_shader_stub SHARED)

target_sources(pixel_shader_stub 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/pixel_shader_stub.cpp
)

target_link_libraries(pixel_shader_stub
    PRIVATE 
        msgpu::interface::shader
)

target_link_libraries(vertex_shader_stub
    PRIVATE 
        msgpu::interface::shader
)
