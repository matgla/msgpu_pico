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

from cpp_utils import CppUtils

from utils import is_message

class CppHeaderGenerator:

    id_counter = 0

    def __init__(self, filename):
        self.filename = filename
        self.generated_messages = []

    def get_messages(self):
        return self.generated_messages

    def get_output_filename(self):
        return self.filename + ".hpp"

    def generate_file_header(self):
        self.output.write(CppUtils.get_header())

    def convert_include(self, t):
        return t.replace(".th", ".hpp")

    def convert_type(self, t):
    # extended type
        if not hasattr(t, "name"):
            name = t.type.name 
        else: 
            name = t.name
       
        
        if name == "uint8":
            return "uint8_t"
        if name == "uint16":
            return "uint16_t"
        if name == "uint32":
            return "uint32_t"
        if name == "float":
            return "float"
        if name == "double":
            return "double"
        if name == "void":
            return "void"

        if not hasattr(t, "name"):
            return t.type.name 


        raise RuntimeError("Conversion not known for: " + t.name)

    def generate_includes(self):
        for include in self.includes:
            self.output.write("#include " + self.convert_include(include) + "\n")
        self.output.write("\n")

    def generate_structure(self, obj, is_msg):
        fields = [] 
        for key, value in obj.lookup.items():
            if value.bits:
                if key != list(obj.lookup)[-1]:
                    fields.append("    {type} {name} : {bits};\n".format(type=self.convert_type(value.type), name=key,
                        bits=value.bits))
                else:
                    fields.append("    {type} {name} : {bits};".format(type=self.convert_type(value.type), name=key,
                        bits=value.bits))
            else:
                if not hasattr(value.type, "count"):
                    if key != list(obj.lookup)[-1]:
                        fields.append("    {type} {name};\n".format(type=self.convert_type(value.type), name=key))
                    else:
                        fields.append("    {type} {name};".format(type=self.convert_type(value.type), name=key))
                else: 
                    if key != list(obj.lookup)[-1]:
                        fields.append("    {type} {name}[{count}];\n".format(type=self.convert_type(value.type), name=key,
                            count=value.type.count))
                    else:
                        fields.append("    {type} {name}[{count}];".format(type=self.convert_type(value.type), name=key,
                            count=value.type.count))


        s = """ 
struct __attribute__((packed, aligned(1))) {name}
{{
""".format(name = obj.name)
        s = s + "".join(fields)
        if is_msg:
            s = s + """
    constexpr static uint8_t id = {id};
""".format(id = CppHeaderGenerator.id_counter)
        s = s + """ 
}};
""".format() 

        self.output.write(s)

    def generate_enum(self, obj):
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
""".format(type = self.convert_type(obj.type), name = obj.name)
        s = s + "".join(values)  

        s = s + """
}};
""".format()
    #    s = "".join([f"{i}" for i in range(0, 10)])
        self.output.write(s)


    def generate(self, items, output_path, includes, original_file):
        with open(output_path, "w") as output:
            self.output = output 
            self.includes = includes 

            self.generate_file_header() 
            self.generate_includes() 

            for key, v in items.items():
                if hasattr(v, "values"):
                    self.generate_enum(v)
                if hasattr(v, "lookup"):
                    is_msg = is_message(original_file, key)
                    self.generate_structure(v, is_msg)
                    if is_msg:
                        self.generated_messages.append({"name": key, "id": CppHeaderGenerator.id_counter})
                        CppHeaderGenerator.id_counter = CppHeaderGenerator.id_counter + 1

