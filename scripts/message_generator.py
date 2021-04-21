#!/bin/python3

import argparse 
import pathlib 
import copy 
import os 
import re 

from dissect import cstruct

parser = argparse.ArgumentParser("Messages generator for MSGPU")

parser.add_argument("--enable-cpp", action="store_true", dest="cpp", help="Enable generation for C++")
parser.add_argument("--enable-python", action="store_true", dest="python", help="Enable generator for Python")
parser.add_argument("--input", action="store", dest="input", help="Path to messages declarations", required=True)
parser.add_argument("--output-dir", action="store", dest="output", help="Directory where messages should be generated",
        required=True)

args = parser.parse_args()

generated_files = []
generated_messages = []
original_sources = [] 

def generate_header(file):
    file.write(""" 
// ********************************
// * FILE AUTOMATICALLY GENERATED *
// ********************************
// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

""")

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


def convert_include(t):
    return t.replace(".th", ".hpp")

def convert_type(t):
    # extended type
    if not hasattr(t, "name"):
        return t.type.name 

    if t.name == "uint8":
        return "uint8_t"
    if t.name == "uint16":
        return "uint16_t"
    if t.name == "uint32":
        return "uint32_t"
    if t.name == "void":
        return "void"

    raise RuntimeError("Conversion not known for: " + t.name)

def generate_enum(obj, file):
    values = [] 
    for key, value in obj.values.items(): 
        if key != list(obj.values)[-1]:
            values.append("    {key} = {value},\n".format(key = key, value = value))
        else:
            values.append("    {key} = {value}".format(key = key, value = value))
       #[f"{type} = ${value}" for i in range(0, 10)]
    #print (values)
    s = """enum {name} : {type}
{{
""".format(type = convert_type(obj.type), name = obj.name)
    s = s + "".join(values)  

    s = s + """
}};
""".format()
#    s = "".join([f"{i}" for i in range(0, 10)])
    file.write(s)

def generate_structure(obj, file):
    fields = [] 
    for key, value in obj.lookup.items():
        if value.bits:
            if key != list(obj.lookup)[-1]:
                fields.append("    {type} {name} : {bits};\n".format(type=convert_type(value.type), name=key,
                    bits=value.bits))
            else:
                fields.append("    {type} {name} : {bits};".format(type=convert_type(value.type), name=key,
                    bits=value.bits))
        else:
            if not hasattr(value.type, "count"):
                if key != list(obj.lookup)[-1]:
                    fields.append("    {type} {name};\n".format(type=convert_type(value.type), name=key))
                else:
                    fields.append("    {type} {name};".format(type=convert_type(value.type), name=key))
            else: 
                if key != list(obj.lookup)[-1]:
                    fields.append("    {type} {name}[{count}];\n".format(type=convert_type(value.type), name=key,
                        count=value.type.count))
                else:
                    fields.append("    {type} {name}[{count}];".format(type=convert_type(value.type), name=key,
                        count=value.type.count))


    s = """ 
struct {name}
{{
""".format(name = obj.name)
    s = s + "".join(fields)
    s = s + """
}};
""".format() 

    file.write(s)
    
    generated_messages.append(obj.name)

def generate_includes(file, includes):
    for include in includes:
        file.write("#include " + convert_include(include) + "\n")
    file.write("\n")

def generate_message(items, file, includes):
    generate_header(file)
    generate_includes(file, includes)
    for k, v in items.items():
        if hasattr(v, "values"):
            generate_enum(v, file)
        if hasattr(v, "lookup"):
            generate_structure(v, file)

def get_includes(file):
    #data.find("/*
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

def generate_cmakelists(file):
    generate_cmakelists_header(file)
    file.write("""
add_library(msgpu_messages INTERFACE)

target_sources(msgpu_messages 
    INTERFACE
""") 

    for f in generated_files: 
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

    for f in original_sources:
        file.write("set_property (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS " + f + ")\n")


def generate_messages_list(file):
    generate_header(file) 
    file.write("#include <cstdint>\n\n")

    file.write("enum class Messages : uint8_t\n")
    file.write("{\n")
    i = 0
    for message in generated_messages:
        file.write("    " + message + " = " + str(i))
        i = i + 1
        if message != generated_messages[-1]:
            file.write(",\n")
    file.write("\n};\n") 

print("Searching messages under: ", args.input)
if args.cpp:
    print("-- enabled CPP")

if args.python:
    print("-- enabled python")

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

    output_path = args.output + "/" + "messages"
    if not os.path.exists(output_path):
        os.makedirs(output_path)

    with open(output_path + "/" + path.stem + ".hpp", "w") as output: 
        generated_files.append(path.stem + ".hpp") 
        generate_message(parsed_types, output, includes)

with open(args.output + "/CMakeLists.txt", "w") as output: 
    generate_cmakelists(output)

with open(args.output + "/messages/messages.hpp", "w") as output:
    generate_messages_list(output)

