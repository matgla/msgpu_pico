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

#add_definitions(-DPICO_SCANVIDEO_COLOR_PIN_COUNT=12
#    -DPICO_SCANVIDEO_DPI_PIXEL_GSHIFT=4
#    -DPICO_SCANVIDEO_DPI_PIXEL_BSHIFT=8
#    -DPICO_SCANVIDEO_DPI_PIXEL_RCOUNT=4
#    -DPICO_SCANVIDEO_DPI_PIXEL_GCOUNT=4
#    -DPICO_SCANVIDEO_DPI_PIXEL_BCOUNT=4
#    -DPICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=4
#    -DPICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=20
#    -DPICO_SCANVIDEO_PLANE1_VARIABLE_FRAGMENT_DMA=1
#)

add_library(msgpu_arch STATIC)

target_sources(msgpu_arch 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/board.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/qspi.cpp
)

target_include_directories(msgpu_arch 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(generator)
add_subdirectory(qspi) 

target_link_libraries(msgpu_arch 
    PUBLIC 
        pico_stdlib
        pico_scanvideo
        pico_scanvideo_dpi
        pico_multicore
        msgpu_interface

        eul_functional 
    PRIVATE
        eul_container
        msgpu_arch_pico_qspi
)


target_compile_options(msgpu_arch PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g -fvar-tracking -fvar-tracking-assignments>
    $<$<CONFIG:RELEASE>:-Os>
)


#find_package(msgui REQUIRED)

#add_subdirectory(config)
#add_subdirectory(disk)
#add_subdirectory(interfaces)
#add_subdirectory(generator)
#add_subdirectory(memory)
#add_subdirectory(modes)
#add_subdirectory(processor)

#find_package(littlefs REQUIRED)

#target_link_libraries(msgpu
#    PRIVATE 
#        pico_stdlib
#        pico_scanvideo_dpi
#        pico_multicore
#        msgui
#        msgpu_generator
#        msgpu_interfaces 
#        msgpu_modes 
#        msgpu_processor
#        msgpu_video_ram
#        msgpu_disk
#        msgpu_config
#)

