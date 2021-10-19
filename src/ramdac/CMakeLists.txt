# This file is part of msgpu project.
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

add_executable(msgpu_ramdac)

target_sources(msgpu_ramdac 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/app.hpp
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_subdirectory(arch)
add_subdirectory(renderer)

target_link_libraries(msgpu_ramdac 
    PRIVATE 
        msgpu_arch
        msgpu_arch_config_ramdac
        msgpu_arch_i2c
        msgpu_lib
        msgpu_memory
        msgpu_renderer
        common_flags
)

if (${MSGPU_ARCH_VERSION} STREQUAL "pico")
    pico_enable_stdio_usb(msgpu_ramdac 0)
    pico_enable_stdio_uart(msgpu_ramdac 1)

    pico_add_extra_outputs(msgpu_ramdac)
elseif (${MSGPU_ARCH_VERSION} STREQUAL "x86_64" AND NOT ENABLE_ST_TESTS)
    add_custom_command(TARGET msgpu_ramdac 
        POST_BUILD 
        COMMAND 
            echo "Enable scheduler options for: $<TARGET_FILE:msgpu_ramdac>"  && 
            sudo setcap cap_sys_nice=ep $<TARGET_FILE:msgpu_ramdac> 
    )
endif ()



