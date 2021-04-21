# This file is part of MS GPU project.
# Copyright (C) 2020 Mateusz Stadnik
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

find_program (virtualenv_exec virtualenv)

if (NOT virtualenv_exec)
    message (FATAL_ERROR "Can't find virtualenv")
endif ()

set (generator_venv ${CMAKE_CURRENT_BINARY_DIR}/venv)

file (GLOB virtualenv_file_stamp ${CMAKE_CURRENT_BINARY_DIR}/venv.stamp)

if (NOT virtualenv_file_stamp)
    message ("Configure virtualenv: ${generator_venv}")

    execute_process(
        COMMAND ${virtualenv_exec} -p python3 venv 
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_ERROR_IS_FATAL ANY 
    )

    execute_process(
        COMMAND ${generator_venv}/bin/pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt --upgrade -q -q -q 
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_ERROR_IS_FATAL ANY 
    )

    execute_process(
        COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/venv.stamp 
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endif ()


execute_process(
    COMMAND ${generator_venv}/bin/python3 ${PROJECT_SOURCE_DIR}/scripts/message_generator.py 
    --input ${PROJECT_SOURCE_DIR}/messages 
    --enable-cpp 
    --output-dir ${PROJECT_BINARY_DIR}/messages
    COMMAND_ECHO STDOUT 
COMMAND_ERROR_IS_FATAL ANY
)


