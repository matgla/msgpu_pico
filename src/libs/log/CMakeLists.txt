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

add_library(msgpu_log INTERFACE)
add_library(msgpu::log ALIAS msgpu_log)

target_sources(msgpu_log 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include/log/log.hpp
)

target_compile_options(msgpu_log 
    INTERFACE 
    -DLOG_TRACE=ON 
    -DLOG_ENABLE_COLORS=ON
)

target_include_directories(msgpu_log 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

