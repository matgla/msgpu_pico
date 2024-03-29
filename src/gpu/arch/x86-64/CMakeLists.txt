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

add_library(msgpu_arch_config_gpu STATIC)

target_sources(msgpu_arch_config_gpu 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/pins_config.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/qspi_config.hpp
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/board.cpp
)

target_include_directories(msgpu_arch_config_gpu
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(msgpu_arch_config_gpu
    PRIVATE 
        msgpu_arch_interface
        msgpu_arch
)
