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

add_executable(msgpu_ut_io)

target_sources(msgpu_ut_io 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_point_tests.cpp
)

target_link_libraries(msgpu_ut_io 
    PRIVATE 
        gtest 
        gmock 
        gtest_main 

        msgpu_io
        msgpu_arch

        common_flags
)


add_test (io msgpu_ut_io)
add_dependencies (check msgpu_ut_io)

