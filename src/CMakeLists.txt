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

if (USE_SIMULATION)
    add_subdirectory(arch/x86)
else () 
    add_subdirectory(arch/pico)
endif ()

add_executable(msgpu) 

target_sources(msgpu 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp 
)

target_compile_options(msgpu
    PRIVATE 
        $<$<COMPILE_LANGUAGE:CXX>:-Werror -Wall -Wextra -Wpedantic>
        $<$<CONFIG:DEBUG>:-Og -g>
        $<$<CONFIG:RELEASE>:-Os>
)

target_link_libraries(msgpu 
    PRIVATE 
        msgpu_interface 
        msgpu_arch 
)

