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

class PythonFileGenerator:
    def __init__(self, filename):
        self.filename = filename



class PythonGenerator:
    def __init__(self):
        self.generated = []

    def get_output_directory(self):
        return "py"

    def create_generator_for(self, filename):
        return self 

    def get_output_filename(self):
        return ""#self.filename.stem + ".py"

    def generate(self, parsed_types, output_path, includes, raw_types):
        self.generated.append({
            "path": output_path,
            "includes": includes, 
            "code": raw_types
        })


    def resolve_includes(self):
        for generated in self.generated:
            print (generated)

    def generate_library_artifacts(self):
        self.resolve_includes()
