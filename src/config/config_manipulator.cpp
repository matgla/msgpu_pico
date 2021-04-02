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

#include "config/config_manipulator.hpp"

namespace config 
{

ConfigManipulator::ConfigManipulator(disk::Disk& disk, const std::string_view& file)
    : disk_(disk)
    , file_(file) 
    , opened_(false)
{
}

void ConfigManipulator::set_parameter(const std::string_view& parameter, const std::string_view& value)
{
    if (!opened_) return;
    if (parameter.size() + value.size() + 3 >= sizeof(line_buffer_)) 
    {
        return;
    }
    printf("Writing to file: %s%s%s\n", parameter.data(), "=", value.data());
    lfs_file_write(&disk_.get_lfs(), &file_handle_, parameter.data(), parameter.size());
    lfs_file_write(&disk_.get_lfs(), &file_handle_, "=", 1);
    lfs_file_write(&disk_.get_lfs(), &file_handle_, value.data(), value.size());
    lfs_file_write(&disk_.get_lfs(), &file_handle_, "\n", 1);
}

void ConfigManipulator::open() 
{
    printf("Opening file\n");
    opened_ = true;
    lfs_file_open(&disk_.get_lfs(), &file_handle_, file_.data(), LFS_O_RDWR | LFS_O_CREAT);
}

void ConfigManipulator::close() 
{
    printf("Closing file\n");
    opened_ = false;
    lfs_file_close(&disk_.get_lfs(), &file_handle_);
}

void ConfigManipulator::print() 
{
    printf("Configuration:\n");
    if (!opened_) return;
    lfs_file_rewind(&disk_.get_lfs(), &file_handle_);
    while (read_line() != EOF)
    {
        printf("  %s\n", line_buffer_);
    }
}

int ConfigManipulator::read_line() 
{
    if (!opened_) return EOF;
    char byte = 0;
    int index = 0;
    while (byte != EOF && byte != '\n')
    {
        lfs_size_t s = lfs_file_read(&disk_.get_lfs(), &file_handle_, &byte, sizeof(char));
        printf("Readed (%c) status: %d\n", byte, s);
        if (s <= 0)
        {
            if (index == 0) return EOF;
            return index; 
        }
        
        line_buffer_[index] = byte; 
        if (index >= (sizeof(line_buffer_) - 1))
        {
            return index;
        }
         
    }
    return EOF;
}

} // namespace config

