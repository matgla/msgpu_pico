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

#include <string_view>

#include "disk/disk.hpp"

namespace config 
{

class ConfigManipulator
{
public:
    ConfigManipulator(disk::Disk& disk, const std::string_view& file);

    void set_parameter(const std::string_view& parameter, const std::string_view& value);

    void print();
    void open(); 
    void close(); 
private:
    int read_line();

    bool opened_;
    char line_buffer_[255];  
    const std::string_view file_;
    disk::Disk& disk_;
    lfs_file_t file_handle_;
};

} // namespace config

