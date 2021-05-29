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

add_library(msgpu_mode INTERFACE)

target_sources(msgpu_mode
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/modes.hpp
)


target_link_libraries(msgpu_mode 
#    PUBLIC 
#        msgpu_messages
#        eul_functional
#    PRIVATE 
INTERFACE 
    common_flags
    )
#        common_flags
#)

target_include_directories(msgpu_mode
    INTERFACE 
        ${PROJECT_SOURCE_DIR}/src
)
