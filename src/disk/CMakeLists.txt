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

add_library(msgpu_disk)

target_sources(msgpu_disk
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/disk.hpp

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/disk.cpp
)

target_link_libraries(msgpu_disk
    PUBLIC
        littlefs
)

target_compile_options(msgpu_disk PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)

target_include_directories(msgpu_disk
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src
    PRIVATE 
        $<TARGET_PROPERTY:pico_stdlib,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:hardware_flash,INTERFACE_INCLUDE_DIRECTORIES>
)
