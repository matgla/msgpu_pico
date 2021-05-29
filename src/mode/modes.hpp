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

#include <variant>


namespace msgpu::mode 
{

template <typename... SupportedModes>
class Modes 
{
public:
    template <typename Mode, typename... Args>
    void switch_to(Args&&... args)
    {
        mode_.template emplace<Mode>(args...);
    }

    template <typename Message>
    bool process(const Message& message)
    {
        return std::visit([&message](auto&& mode) {
            constexpr bool has_process = requires() {
                mode.process(message);
            };

            if constexpr (has_process)
            {
                mode.process(message);
                return true;
            }
            return false;
        }, mode_);
    }

private:
    std::variant<std::monostate, SupportedModes...> mode_;
};

template <typename... SupportedModes>
struct ModesFactory
{
    template <typename Mode>
    constexpr static ModesFactory<SupportedModes..., Mode> add_mode()
    {
        return {};
    }

    constexpr static Modes<SupportedModes...> create() 
    {
        return Modes<SupportedModes...>{};
    }
};

} // namespace msgpu::mode

