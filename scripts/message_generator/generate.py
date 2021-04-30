#!/bin/python3

import argparse 
import pathlib 
import copy 
import os 
import re 

from dissect import cstruct

from cpp_generator import CppGenerator 
from python_generator import PythonGenerator

parser = argparse.ArgumentParser("Messages generator for MSGPU")

parser.add_argument("--enable-cpp", action="store_true", dest="cpp", help="Enable generation for C++")
parser.add_argument("--enable-python", action="store_true", dest="python", help="Enable generator for Python")
parser.add_argument("--input", action="store", dest="input", help="Path to messages declarations", required=True)
parser.add_argument("--output-dir", action="store", dest="output", help="Directory where messages should be generated",
        required=True)

args = parser.parse_args()

def generate_cmakelists_header(file):
    file.write("""#  ********************************
#  * FILE AUTOMATICALLY GENERATED *
#  ********************************
#  This file is part of msgpu project.
#  Copyright (C) 2021 Mateusz Stadnik
# 
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

""")

def get_includes(file):
    include_regex = re.compile("/*.*include:") 
    for line in file.readlines():
        result = include_regex.search(line)
        
        if result: 
            includes_list = line.strip(result.group(0))
            includes_list = includes_list.replace("\n", "")
            includes_list = includes_list.replace("\r", "")
            includes_list = includes_list.replace(" ", "")
            includes_list = includes_list.replace("*/", "")

            return includes_list.split(",") 
    return []

print("Searching messages under: ", args.input)

generators = []
original_sources = []

if args.cpp:
    print("-- enabled CPP")
    generators.append(CppGenerator(args.output, args.input))

if args.python:
    print("-- enabled python")
    generators.append(PythonGenerator(args.output))

for path in pathlib.Path(args.input).rglob("*.th"):
    original_sources.append(str(path)) 
    cparser = cstruct.cstruct()

    with open(path, "r") as file:
        readed = file.read()
        file.seek(0)
        includes = get_includes(file)

    for dependant in includes:
        if dependant.find(".th") != -1:
            dependant = dependant.replace("\"", "")
            with open(args.input + "/" + dependant, "r") as f: 
                cparser.load(f.read())
    
    original_types = copy.copy(cparser.typedefs)
    
    cparser.load(readed)
 
    parsed_types = copy.copy(cparser.typedefs)
    for type in original_types:
        parsed_types.pop(type)

    for generator in generators:
        output_path = generator.get_output_directory()

        if not os.path.exists(output_path):
            os.makedirs(output_path)
        
        file_generator = generator.create_generator_for(path)

        output_filepath = file_generator.get_output_filename()
        file_generator.generate(parsed_types, output_path + "/" + output_filepath, includes, readed)

for generator in generators:
    generator.generate_library_artifacts()

# with open(args.output + "/messages/messages.hpp", "w") as output:
#     generate_messages_list(output)

