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

add_library(msgpu_interfaces)

target_sources(msgpu_interfaces
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/usart.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spi.hpp

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/usart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spi.cpp
)

target_compile_options(msgpu_interfaces PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)

target_link_libraries(msgpu_interfaces 
    PRIVATE
        eul
)

target_include_directories(msgpu_interfaces
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)
