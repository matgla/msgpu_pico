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

namespace config 
{

enum class Mode 
{
    mode_1024x768_63,
    mode_800x600_60,
    mode_640x480_60,
    mode_400x240_60,
    mode_320x240_60,
    mode_256x192_50,
    mode_720p_60
};

class Config 
{
public:
    Mode mode;
};


} // namespace config

