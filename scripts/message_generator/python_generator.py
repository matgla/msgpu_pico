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

import datetime

from utils import is_message

class PythonFileGenerator:
    def __init__(self, filename):
        self.filename = filename
        self.generated = {} 
        self.files = []

    def get_output_filename(self):
        return self.filename.stem + ".py"

    def generate(self, parsed_types, output_path, includes, raw_types):
        self.generated = {
            "source": self.filename.name,
            "path": output_path,
            "includes": includes, 
            "code": raw_types,
            "types": parsed_types
        }
        
        for k, v in parsed_types.items():
            if hasattr(v, "lookup"):
                self.files.append(k) 

    def get(self):
        return self.generated
    
    def get_messages(self):
        return self.files

class PythonGenerator:
    OUTPUT_DIR = "py"
    def __init__(self, output_directory):
        self.generators = []
        self.output_directory = output_directory

    def get_output_directory(self):
        return self.output_directory + "/" + PythonGenerator.OUTPUT_DIR + "/messages"

    def create_generator_for(self, filename):
        generator = PythonFileGenerator(filename) 
        self.generators.append(generator)
        return generator

    def get_source(self, filename):
        filename = filename.replace('"', '')
        for generated in self.generators:
            print(generated.get()["source"])
            if generated.get()["source"] == filename:
                return generated.get()["code"] 
        raise RuntimeError("Couldn't find include: " + filename)

    def generate_library_artifacts(self):
        for generated in self.generators:
            output_path = generated.get()["path"]

            with open(output_path, "w") as file:
                file.write("message_data = \"\"\"\n")
                for include in generated.get()["includes"]:
                    file.write(self.get_source(include))
                file.write(generated.get()["code"])
                file.write("\"\"\"\n")

                file.write("from dissect import cstruct\n") 
                file.write("parsed = cstruct.cstruct(align=4)\n")
                file.write("parsed.load(message_data)\n")
                for obj in generated.get()["types"]:
                    file.write(obj + " = parsed." + obj + "\n")
        messages_path = self.get_output_directory() + "/messages.py" 
        with open(messages_path, "w") as file: 
            file.write("class Messages:\n")
            i = 0
            generated_messages = []
            for generated in self.generators:
                msgs = generated.get_messages() 
                for msg in msgs:
                    if is_message(generated.get()["code"], msg):
                        generated_messages.append(msg)
            for message in generated_messages:
                file.write("    " + message + " = " + str(i) + "\n")
                i = i + 1
