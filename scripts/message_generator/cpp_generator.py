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

from cpp_header_generator import CppHeaderGenerator
from cpp_utils import CppUtils 

import pathlib 

class CppGenerator: 
    OUTPUT_DIR = "cpp"

    def __init__(self, output_directory, input_directory):
        self.input_path = pathlib.Path(input_directory).name

        self.generated_files = []
        self.original_sources = []
        self.generators = []
        self.output_directory = output_directory

    def create_generator_for(self, filename):
        file_generator = CppHeaderGenerator(filename.stem)
        print(filename)
        self.original_sources.append(str(filename.name))
        self.generated_files.append(file_generator.get_output_filename())

        self.generators.append(file_generator)
        return file_generator

    def get_output_directory(self):
        return self.output_directory + "/" + CppGenerator.OUTPUT_DIR + "/messages"

    def generate_cmakelists(self, file):
        file.write("""
add_library(msgpu_messages INTERFACE)

target_sources(msgpu_messages 
    INTERFACE
""") 

        for f in self.generated_files: 
            file.write("    ${CMAKE_CURRENT_SOURCE_DIR}/messages/" + f + "\n")

        file.write("""
)
""")

        file.write(""" 
target_include_directories(msgpu_messages 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR} 
)
""")

        for f in self.original_sources:
            file.write("set_property (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/" +
                    self.input_path + "/" + f + ")\n")

    
    def generate_messages_list(self, file):
        file.write(CppUtils.get_header())
        file.write("#include <cstdint>\n\n")

        file.write("enum class Messages : uint8_t\n")
        file.write("{\n")
       
        generated_messages = []
        for generator in self.generators:
            generated_messages.extend(generator.get_messages())
        for message in generated_messages:
            file.write("    " + message["name"] + " = " + str(message["id"]))
            if message != generated_messages[-1]:
                file.write(",\n")
        file.write("\n};\n") 


    def generate_library_artifacts(self):

        with open(self.output_directory + "/" + CppGenerator.OUTPUT_DIR + "/CMakeLists.txt", 'w') as file:
            self.generate_cmakelists(file) 

        with open(self.output_directory + "/" + CppGenerator.OUTPUT_DIR + "/messages/messages.hpp", "w") as file: 
            self.generate_messages_list(file)

