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

add_library(msgpu_arch OBJECT) # must be OBJECT library to make lazy linking

target_sources(msgpu_arch 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/panic.hpp
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/board.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/qspi.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_dma.cpp
)

target_include_directories(msgpu_arch 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(generator)
add_subdirectory(i2c)
add_subdirectory(qspi) 

target_include_directories(msgpu_arch SYSTEM
    PUBLIC 
        $<TARGET_PROPERTY:pico_stdio,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:hardware_resets,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:pico_scanvideo_dpi,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:pico_scanvideo,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:pico_multicore,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:pico_platform_headers,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:pico_stdlib_headers,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:pico_binary_info_headers,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:hardware_clocks_headers,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:hardware_irq_headers,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:hardware_pio_headers,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:hardware_dma_headers,INTERFACE_INCLUDE_DIRECTORIES>



)

target_link_libraries(msgpu_arch 
    PUBLIC 
        msgpu_arch_interface
        eul_functional 

    PRIVATE
 
        msgpu_generator
        eul_container
        msgpu_arch_pico_qspi
)

target_compile_options(msgpu_arch PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g -fvar-tracking -fvar-tracking-assignments>
    $<$<CONFIG:RELEASE>:-Os>
)

add_library (msgpu_config INTERFACE)
target_include_directories (msgpu_config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library (msgpu_lib INTERFACE)
target_link_libraries (msgpu_lib
    INTERFACE 
        pico_stdlib
        pico_scanvideo
        pico_scanvideo_dpi
        hardware_irq
        hardware_clocks
        hardware_dma
        hardware_i2c
        pico_multicore
        pico_runtime
)

