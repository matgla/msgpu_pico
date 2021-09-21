// This file is part of msgpu project.
// Copyright (C) 2021 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it is under the terms of the GNU General Public License as published by
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

#include <cstdio>

#include <string_view>

namespace msgpu::log
{

namespace
{

#ifdef LOG_ENABLE_COLORS
constexpr inline bool colors_enabled = true;
#else
constexpr inline bool colors_enabled = false;
#endif

#ifdef LOG_TRACE
#define LOG_DEBUG
constexpr inline bool enable_trace = true;
#else
constexpr inline bool enable_trace   = false;
#endif

#ifdef LOG_DEBUG
#define LOG_INFO
constexpr inline bool enable_debug = true;
#else
constexpr inline bool enable_debug   = false;
#endif

#ifdef LOG_INFO
#define LOG_WARNING
constexpr inline bool enable_info = true;
#else
constexpr inline bool enable_info    = false;
#endif

#ifdef LOG_WARNING
#define LOG_ERROR
constexpr inline bool enable_warning = true;
#else
constexpr inline bool enable_warning = false;
#endif

#ifdef LOG_ERROR
constexpr inline bool enable_error = true;
#else
constexpr inline bool enable_error   = false;
#endif

} // namespace

class Log
{
  private:
    enum class Colors
    {
        Yellow,
        Red,
        Cyan,
        Blue,
        White,
        Reset
    };

  public:
    template <typename... Args>
    static void trace(Args &&...args)
    {
        if constexpr (enable_trace)
        {
            print_prefix("[TRC]", Colors::Blue);
            printf(args...);
            printf("\n");
        }
    }

    template <typename... Args>
    static void debug(Args &&...args)
    {
        if constexpr (enable_debug)
        {
            print_prefix("[DBG]", Colors::Cyan);
            printf(args...);
            printf("\n");
        }
    }

    template <typename... Args>
    static void info(Args &&...args)
    {
        if constexpr (enable_info)
        {
            print_prefix("[INF]", Colors::White);
            printf(args...);
            printf("\n");
        }
    }

    template <typename... Args>
    static void warning(Args &&...args)
    {
        if constexpr (enable_warning)
        {
            print_prefix("[WRN]", Colors::Yellow);
            printf(args...);
            printf("\n");
        }
    }

    template <typename... Args>
    static void error(Args &&...args)
    {
        if constexpr (enable_error)
        {
            print_prefix("[ERR]", Colors::Red);
            printf(args...);
            printf("\n");
        }
    }

  private:
    static void print_prefix(std::string_view text, Colors color)
    {
        if constexpr (colors_enabled)
        {
            set_color(color);
        }
        printf("%s", text.data());
        printf(" ");

        if constexpr (colors_enabled)
        {
            set_color(Colors::Reset);
        }
    }

    static void set_color(Colors color)
    {
        switch (color)
        {
        case Colors::Blue:
            printf("\x1b[34m");
            break;
        case Colors::Cyan:
            printf("\x1b[36m");
            break;
        case Colors::Red:
            printf("\x1b[31m");
            break;
        case Colors::Yellow:
            printf("\x1b[33m");
            break;
        case Colors::White:
            printf("\x1b[37m");
            break;
        case Colors::Reset:
            printf("\x1b[0m");
        }
    }
};

} // namespace msgpu::log
