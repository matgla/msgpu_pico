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

add_executable(msgpu_ut_mode)

target_sources(msgpu_ut_mode
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/modes_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/programs_tests.cpp
)

target_link_libraries(msgpu_ut_mode
    PRIVATE 
        gtest
        gmock 
        gtest_main 

        msgpu_mode

        common_flags
)

target_link_options(msgpu_ut_mode 
    PRIVATE 
        -rdynamic
)

add_test(mode msgpu_ut_mode)
add_dependencies(check msgpu_ut_mode)

add_library(vertex_shader_test SHARED)

target_sources(vertex_shader_test 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/vertex_shader_stub.cpp
)

add_library(fragment_shader_test SHARED)

target_sources(fragment_shader_test 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/fragment_shader_stub.cpp
)

target_compile_definitions(msgpu_ut_mode 
    PRIVATE 
        -DVERTEX_SHADER_PATH="$<TARGET_FILE:vertex_shader_test>"
        -DFRAGMENT_SHADER_PATH="$<TARGET_FILE:fragment_shader_test>"
)

add_dependencies(msgpu_ut_mode fragment_shader_test)
add_dependencies(msgpu_ut_mode vertex_shader_test)
