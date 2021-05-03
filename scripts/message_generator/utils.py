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

import re 
import io 

def is_message(file, name):
    not_message_regex = re.compile("/*.*not_message:")
    
    for line in io.StringIO(file).readlines():
        result = not_message_regex.search(line)
        if result:
            not_messages = line.strip(result.group(0))
            not_messages = not_messages.split(" ")
            not_messages = not_messages[0]
            not_messages = not_messages.split(",")
            if name in not_messages: 
                return False

    return True
