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

add_library(msgpu_modes INTERFACE)

target_sources(msgpu_modes
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/buffer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/colors.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mode_base.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/types.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/text/text_mode.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/text/80x30_16.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/text/40x30_16.hpp
)

target_link_libraries(msgpu_modes
    INTERFACE 
        msgui 
        msgpu_arch_interface
        msgpu_arch
        msgpu_generator_interface
        msgpu_generator
        msgpu_messages
        eul_math
)

target_compile_options(msgpu_modes INTERFACE 
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)

target_include_directories(msgpu_modes 
    INTERFACE 
        ${PROJECT_SOURCE_DIR}/src
)

