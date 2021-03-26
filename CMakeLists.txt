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

set (PICO_COPY_TO_RAM ON)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${mspkg_SOURCE_DIR}/cmake)
include (mspkg)

setup_mspkg(${mspkg_SOURCE_DIR})

find_package(pico-sdk REQUIRED)

include (cmake/preload/toolchains/pico_arm_gcc)

include (pico_sdk_init)
enable_language(ASM C CXX) 

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pico-extras REQUIRED)
include (external/pico_extras_import)

pico_sdk_init()

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
