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
# You should have received a copy of the GNU General Public License # along with this program.  If not, see <https://www.gnu.org/licenses/>.

find_package(msgpu_interface REQUIRED)

add_subdirectory(arch)
add_subdirectory(buffers)
add_subdirectory(io)
add_subdirectory(mode)
add_subdirectory(processor)

add_executable(msgpu) 

target_sources(msgpu 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp 
)

target_compile_options(msgpu
    PRIVATE 
        -rdynamic
        $<$<CONFIG:DEBUG>:-Og -g>
        $<$<CONFIG:RELEASE>:-Os>
)

target_link_libraries(msgpu PUBLIC -rdynamic)

target_link_libraries(msgpu 
    PRIVATE 
        msgpu::log
        msos_dynamic_linker
        msgpu_arch_interface 
        msgpu_arch_i2c
        msgpu_arch 
        msgpu_lib
        msgpu_memory
        msgpu_arch_config_gpu
        msgpu_mode
        msgpu_generator
        msgpu_processor
        msgpu_messages
        msgui
)

add_dependencies(msgpu api_generator)

if (${MSGPU_ARCH_VERSION} STREQUAL "pico")
    pico_enable_stdio_usb(msgpu 0)
    pico_enable_stdio_uart(msgpu 1)

    pico_add_extra_outputs(msgpu)
elseif (${MSGPU_ARCH_VERSION} STREQUAL "x86_64" AND NOT ENABLE_ST_TESTS)
    add_custom_command(TARGET msgpu 
        POST_BUILD 
        COMMAND 
            echo "Enable scheduler options for: $<TARGET_FILE:msgpu>"  && 
            sudo setcap cap_sys_nice=ep $<TARGET_FILE:msgpu> 
    )
endif ()