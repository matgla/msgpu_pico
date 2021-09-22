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

find_package(gtest REQUIRED)

add_custom_target(check 
    COMMAND GTEST_COLOR=1 ${CMAKE_CTEST_COMMAND} -V)

add_subdirectory(buffers)
add_subdirectory(io)
add_subdirectory(mode)
add_subdirectory(processor)

