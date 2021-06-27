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

add_library(msgpu_generator_interface INTERFACE)

target_sources(msgpu_generator_interface
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/vga.hpp
)

target_compile_options(msgpu_generator_interface INTERFACE 
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g -fvar-tracking -fvar-tracking-assignments>
    $<$<CONFIG:RELEASE>:-Os>
)

target_include_directories(msgpu_generator_interface
    INTERFACE  
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/generator
)

target_link_libraries(msgpu_generator_interface 
    INTERFACE 
        msgpu_config 
)
