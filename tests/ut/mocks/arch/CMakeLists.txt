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

message (STATUS "Configuring arch for UT")

add_library(msgpu_arch STATIC)

target_sources(msgpu_arch 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/config.hpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/sync.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_dma_mocks.hpp
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/board_stubs.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_dma.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_dma_mocks.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_stub.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qspi_stub.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vga_stub.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sync.cpp
)

target_include_directories(msgpu_arch
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(msgpu_arch 
    PUBLIC 
        msgpu_arch_interface
        msgpu_generator_interface
        gmock
)

add_library(msgpu_generator INTERFACE)
target_link_libraries(msgpu_generator INTERFACE msgpu_generator_interface msgpu_arch_interface)
add_library(msgpu_arch_i2c INTERFACE)
add_library(msgpu_lib INTERFACE)
add_library(msgpu_config INTERFACE)
target_include_directories(msgpu_config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(msgpu_arch_config_ramdac INTERFACE) 

target_link_libraries(msgpu_arch_config_ramdac 
    INTERFACE 
        msgpu_arch 
        msgpu_arch_interface
        msgpu_generator
)

