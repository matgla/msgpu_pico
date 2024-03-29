# This file is part of msgpu project.
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

add_library(msgpu_generator OBJECT)

target_sources(msgpu_generator
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/vga.cpp
)

target_link_libraries(msgpu_generator
    PRIVATE 
        msgui
    PUBLIC
        msgpu_generator_interface 
        msgpu_arch_interface
)

target_include_directories(msgpu_generator SYSTEM
    PRIVATE 
        $<TARGET_PROPERTY:pico_multicore,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:pico_stdlib,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:pico_scanvideo_dpi,INTERFACE_INCLUDE_DIRECTORIES>

)

target_compile_options(msgpu_generator PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)

target_include_directories(msgpu_generator PUBLIC 
    ${PROJECT_SOURCE_DIR}/src/gpu
)


