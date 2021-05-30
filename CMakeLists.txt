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

cmake_minimum_required(VERSION 3.9)

project (msgpu NONE) 


find_package (Python3 COMPONENTS Interpreter) 

include(FetchContent)

FetchContent_Declare(
  mspkg
  GIT_REPOSITORY https://github.com/matgla/mspkg.git
  GIT_TAG        master
)

FetchContent_MakeAvailable(mspkg)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${mspkg_SOURCE_DIR}/cmake)
include (mspkg)

setup_mspkg(${mspkg_SOURCE_DIR})

if (NOT USE_SIMULATION AND NOT ENABLE_TESTS)
    message (STATUS "Searching pico sdk")
    find_package(pico-sdk REQUIRED)
    include (cmake/preload/toolchains/pico_arm_gcc)
    include (pico_sdk_init)
endif ()

enable_language(ASM C CXX) 
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT USE_SIMULATION AND NOT ENABLE_TESTS)
    find_package(pico-extras REQUIRED)
    include (external/pico_extras_import)
    pico_sdk_init()
endif ()


set (HAS_CXX20_FLAG ON CACHE INTERNAL "" FORCE)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 20)

add_library(common_flags INTERFACE)

target_compile_options(common_flags
    INTERFACE 
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wfloat-equal -Wshadow -Wundef -Wpointer-arith -Wcast-align -Wconversion -Wall
    -Wpedantic -Wextra -Werror>
    $<$<CONFIG:DEBUG>:-Og -g> 
    $<$<CONFIG:RELEASE>:-Os>
)
#set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/scripts/message_generator.py)

add_definitions(-DPICO_SCANVIDEO_COLOR_PIN_COUNT=8
    -DPICO_SCANVIDEO_DPI_PIXEL_GSHIFT=3
    -DPICO_SCANVIDEO_DPI_PIXEL_BSHIFT=6
    -DPICO_SCANVIDEO_DPI_PIXEL_RCOUNT=3
    -DPICO_SCANVIDEO_DPI_PIXEL_GCOUNT=3
    -DPICO_SCANVIDEO_DPI_PIXEL_BCOUNT=2
    -DPICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=10
    -DPICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=40
    -DPICO_SCANVIDEO_PLANE1_VARIABLE_FRAGMENT_DMA=1
)
find_package(msgpu_interface)
add_subdirectory(arch)

add_subdirectory(include)
add_subdirectory(src)

if (ENABLE_TESTS)
    message(STATUS "Testing: enabled")
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()

#file (CREATE_LINK ${PROJECT_BINARY_DIR}/compile_commands.json ${PROJECT_SOURCE_DIR}/compile_commands.json)
