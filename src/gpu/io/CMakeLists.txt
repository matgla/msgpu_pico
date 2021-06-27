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


add_library(msgpu_io)

target_sources(msgpu_io 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_point.hpp 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_point.cpp 
)

find_package(sml REQUIRED)

get_target_property(sml_include_directories sml INTERFACE_INCLUDE_DIRECTORIES)

add_library(sml_external INTERFACE) 

target_include_directories(sml_external SYSTEM INTERFACE ${sml_include_directories})

add_library(external::sml ALIAS sml_external)

target_link_libraries(msgpu_io 
    PRIVATE
        common_flags
        msgpu_arch
    PUBLIC
        eul_container
        external::sml 
        msgpu_messages
)

target_include_directories(msgpu_io 
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/gpu
)
