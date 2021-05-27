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

add_subdirectory(mocks)

add_executable(msgpu_ut)

target_sources(msgpu_ut 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/io/usart_point_tests.cpp
)

find_package(gtest REQUIRED)

target_link_libraries(msgpu_ut
    PRIVATE 
        gtest 
        gmock 
        gtest_main

        msgpu_arch          
        msgpu_io
)
