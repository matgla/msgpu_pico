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

add_library(msgpu_processor)

target_sources(msgpu_processor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/message_processor.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/handler.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/message_processor.cpp
)


target_link_libraries(msgpu_processor 
    PUBLIC 
        msgpu_messages
        eul_functional
        msgpu_io
    PRIVATE 
        common_flags
)

target_include_directories(msgpu_processor
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/gpu
)
