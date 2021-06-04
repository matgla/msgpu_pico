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

#include <cstdint>
#include <variant>
#include <span>
#include <type_traits>


namespace msgpu::mode 
{

/// @brief Class to collect all graphic processing methods 
///
/// @details 
///   Only one mode may be selected (i.e TextMode 80x30, Graphic Mode 320x240).
///   All available modes are stored in variant, for memory sharing.
///   Each byte is valuable in devices with low memory size. 
///   Class is also responsible for dispatching messages only if it's supported by specific mode.
template <typename... SupportedModes>
class Modes 
{
public:

    /// @brief Changes selected mode to different.
    ///
    /// @tparam Mode - new mode to be used 
    /// @param Args... - arguments needed to call Mode constructor
    template <typename Mode, typename... Args>
    void switch_to(Args&&... args)
    {
        mode_.template emplace<Mode>(args...);
    }

    /// @brief Forwards message to mode only if it's supported. 
    ///
    /// @param message - message to be processed 
    /// 
    /// @returns true - if message was processed, false - if message is not supported in specific mode 
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

    /// @brief Get line buffer from mode 
    ///
    /// @param line - line which should be filled
    ///
    /// @returns std::span<uint8_t> buffer with line data
    std::span<const uint8_t> get_line(std::size_t line) const
    {
        return std::visit([line](auto&& mode) {
            constexpr bool has_get_line = requires() {
                { mode.get_line(line) } -> std::convertible_to<std::span<const uint8_t>>;
            };

            if constexpr (has_get_line)
            {
                return mode.get_line(line);
            }
            return std::span<const uint8_t>{};
        }, mode_);
    }

private:
    std::variant<std::monostate, SupportedModes...> mode_;
};

/// @brief Constructs Modes object with DSL form 
template <typename... SupportedModes>
struct ModesFactory
{

    /// @brief Register mode type for further object construction
    ///
    /// @tparam Mode - registered type for object creation
    template <typename Mode>
    constexpr static ModesFactory<SupportedModes..., Mode> add_mode()
    {
        return {};
    }

    /// @brief Creates Mode object with all registered types.
    constexpr static Modes<SupportedModes...> create() 
    {
        return Modes<SupportedModes...>{};
    }
};

} // namespace msgpu::mode

