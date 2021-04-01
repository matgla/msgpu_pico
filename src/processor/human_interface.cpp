// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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

#include "processor/human_interface.hpp"

#include <cstdio>
#include <algorithm>

#include <eul/mpl/tuples/for_each.hpp>

#include "modes/colors.hpp"


namespace processor
{

template <typename T, typename C>
class Handler
{
public:
    constexpr Handler(std::string_view name, C* object, T data) : handler_(name, std::make_pair(object, data))
    {
    }

    std::pair<std::string_view, std::pair<C*, T>> handler_;
};

template <typename... handlers>
class Handlers
{
public:
    std::tuple<handlers...> handlers_;
};


template <typename... handlers>
Handlers(handlers...) -> Handlers<handlers...>;

HumanInterface::HumanInterface(vga::Mode& mode)
    : position_(0)
    , mode_(&mode)
{
    printf("\n> ");
}

void HumanInterface::process_command()
{
    buffer_[position_] = 0;
    printf("\nProcessing command: %s\n", buffer_);
    position_ = 0;
    to_parse_ = buffer_;

    const auto command = get_next_part();

    const static Handlers handlers {
        Handler{"help", this, &HumanInterface::help}
    };

    eul::mpl::tuples::for_each(handlers.handlers_, [&command](const auto& handler) {
        if (command == handler.handler_.first)
        {
            const auto* object = handler.handler_.second.first;
            (object->*handler.handler_.second.second)();
        }
    });
}

void HumanInterface::process(uint8_t byte)
{
    // backspace
    if (byte == 8)
    {
        mode_->move_cursor(0, -1);
        mode_->write(0);
        mode_->move_cursor(0, -1);

        return;
    }

    // enter
    if (byte == 13)
    {
        mode_->move_cursor(1, 0);
        mode_->set_cursor_column(0);

        return;
    }

    if (escape_code_)
    {
        escape_code_ = false;
        switch (byte)
        {
            case 91:
            {
                cursor_move_ = true;
                return;
            }
        }
    }

    if (cursor_move_)
    {
        cursor_move_ = false;
        switch (byte)
        {
            // left
            case 68:
            {
                mode_->move_cursor(0, -1);
                return;
            }
            // right
            case 67:
            {
                mode_->move_cursor(0, 1);
                return;
            }
            // up
            case 65:
            {
                mode_->move_cursor(-1, 0);
                return;
            }
            // down
            case 66:
            {
                mode_->move_cursor(1, 0);
                return;
            }
            case 0: // reset
            {
                mode_->set_color(colors::white, colors::black);
                return;
            }
            case 30: // fg black
            {
                mode_->set_foreground_color(colors::black);
                return;
            }
            case 31: // fg red
            {
                mode_->set_foreground_color(colors::red);
                return;
            }
            case 32: // fg green
            {
                mode_->set_foreground_color(colors::green);
                return;
            }
            case 33: // fg yellow
            {
                mode_->set_foreground_color(colors::yellow);
                return;
            }
            case 34: // fg blue
            {
                mode_->set_foreground_color(colors::blue);
                return;
            }
            case 35: // fg magenta
            {
                mode_->set_foreground_color(colors::magneta);
                return;
            }
            case 36: // fg cyan
            {
                mode_->set_foreground_color(colors::cyan);
                return;
            }
            case 37: // fg white
            {
                mode_->set_foreground_color(colors::white);
                return;
            }
            case 40: // bg black
            {
                mode_->set_background_color(colors::black);
                return;
            }
            case 41: // bg red
            {
                mode_->set_background_color(colors::red);
                return;
            }
            case 42: // bg green
            {
                mode_->set_background_color(colors::green);
                return;
            }
            case 43: // bg yellow
            {
                mode_->set_background_color(colors::yellow);
                return;
            }
            case 44: // bg blue
            {
                mode_->set_background_color(colors::blue);
                return;
            }
            case 45: // bg magenta
            {
                mode_->set_background_color(colors::magneta);
                return;
            }
            case 46: // bg cyan
            {
                mode_->set_background_color(colors::cyan);
                return;
            }
            case 47: // bg white
            {
                mode_->set_background_color(colors::white);
                return;
            }
        }
    }

    if (byte == 27)
    {
        printf("ESC\n");
        escape_code_ = true;
        return;
    }
    mode_->write(static_cast<char>(byte));

}

void HumanInterface::help() const
{
    printf("Available commands:\n");
    printf("  mode [mode] - select display mode\n");
    printf("To get more information please write help [command].\n");
}

std::string_view HumanInterface::get_next_part()
{
    if (to_parse_.empty())
    {
        return to_parse_;
    }
    to_parse_.remove_prefix(std::min(to_parse_.find_first_not_of(" "), to_parse_.size()));
    const auto next_part = to_parse_.substr(0, std::min(to_parse_.find_first_not_of(" "), to_parse_.size()));
    to_parse_.remove_suffix(next_part.size());
    return next_part;
}

} // namespace processor
