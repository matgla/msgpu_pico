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


add_executable(msgpu) 

add_definitions(-DPICO_SCANVIDEO_COLOR_PIN_COUNT=12
-DPICO_SCANVIDEO_DPI_PIXEL_GSHIFT=4
-DPICO_SCANVIDEO_DPI_PIXEL_BSHIFT=8
-DPICO_SCANVIDEO_DPI_PIXEL_RCOUNT=4
-DPICO_SCANVIDEO_DPI_PIXEL_GCOUNT=4
-DPICO_SCANVIDEO_DPI_PIXEL_BCOUNT=4
-DPICO_SCANVIDEO_PLANE1_VARIABLE_FRAGMENT_DMA=1
)
target_sources(msgpu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp )

find_package(msgui REQUIRED)

add_subdirectory(interfaces)
add_subdirectory(generator)
add_subdirectory(memory)
add_subdirectory(modes)
add_subdirectory(processor)

target_link_libraries(msgpu
    PUBLIC  
        pico_stdlib
        pico_scanvideo_dpi
        pico_multicore
        msgui
        msgpu_generator
        msgpu_interfaces 
        msgpu_modes 
        msgpu_processor
)

pico_enable_stdio_usb(msgpu 1)
pico_enable_stdio_uart(msgpu 0)

pico_add_extra_outputs(msgpu)

