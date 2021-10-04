# This file is part of MS GPU project.
# Copyright (C) 2020 Mateusz Stadnik
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

add_library(msgpu_mode STATIC)

set (include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/mode)
target_sources(msgpu_mode
    PUBLIC  
        ${include_dir}/2d_graphic_mode.hpp
        ${include_dir}/3d_graphic_mode.hpp
        ${include_dir}/buffer.hpp
        ${include_dir}/buffer_generator.hpp
        ${include_dir}/framebuffer.hpp
        ${include_dir}/mode_base.hpp
        ${include_dir}/modes.hpp
        ${include_dir}/text_mode.hpp
        ${include_dir}/program.hpp
        ${include_dir}/programs.hpp 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/programs.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/program.cpp
)


target_link_libraries(msgpu_mode 
    PUBLIC  
        common_flags
        eul_math
        eul_container
        msgpu::gpu::buffers
        msgpu::memory
        msgpu::log
        msos_dynamic_linker
)

target_include_directories(msgpu_mode
    PUBLIC  
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
