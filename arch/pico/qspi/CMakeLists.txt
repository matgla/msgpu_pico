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

add_library (msgpu_arch_pico_qspi)

pico_generate_pio_header(msgpu_arch_pico_qspi ${CMAKE_CURRENT_LIST_DIR}/qspi.pio)

target_sources(msgpu_arch_pico_qspi
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/pio_qspi.h
    PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/pio_qspi.cpp
)

target_link_libraries(msgpu_arch_pico_qspi
    PRIVATE 
        pico_stdlib hardware_pio 
)

target_include_directories(msgpu_arch_pico_qspi 
    PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR}
)

