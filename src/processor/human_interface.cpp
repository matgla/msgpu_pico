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

#include <algorithm>
#include <cstdio>
#include <charconv>

#include <unistd.h> 

#include <eul/mpl/tuples/for_each.hpp>
#include <eul/utils/string.hpp>

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
    Handlers(handlers... h) : handlers_(h...) 
    {
    }

    std::tuple<handlers...> handlers_;
};


template <typename... handlers>
Handlers(handlers...) -> Handlers<handlers...>;

HumanInterface::HumanInterface(vga::Mode& mode)
    : position_(0)
    , mode_(&mode)
    , state_(State::waiting_for_command)
{
    printf("\n> ");
}

void HumanInterface::write()
{
    state_ = State::writing;
}

void HumanInterface::process_command()
{
    printf("\nProcessing command: %s\n", buffer_);
    to_parse_ = buffer_;

    const auto command = get_next_part();

    const static Handlers handlers {
        Handler{"help", this, &HumanInterface::help},
        Handler{"write", this, &HumanInterface::write},
        Handler{"clear", this, &HumanInterface::clear},
        Handler{"mode", this, &HumanInterface::mode}
    };

    eul::mpl::tuples::for_each(handlers.handlers_, [&command](auto& handler) {
        if (command == handler.handler_.first)
        {
            auto* object = handler.handler_.second.first;
            (object->*handler.handler_.second.second)();
        }
    });
}

void HumanInterface::mode()
{
    const auto arg = get_next_part();
   
    int mode;
    std::from_chars(arg.begin(), arg.end(), mode);
    printf("Changing mode to: %d\n", mode);
    
    switch (mode)
    {
        case 1: 
        {
            mode_->switch_to(vga::Modes::Text_80x30_16);
        } break;
        case 2: 
        {
            mode_->switch_to(vga::Modes::Text_40x30_16);
        } break;
        case 3: 
        {
            mode_->switch_to(vga::Modes::Text_40x30_12bit);
        } break;
        case 10: 
        {
            mode_->switch_to(vga::Modes::Graphic_640x480_16);
        } break;
        case 11: 
        {
            mode_->switch_to(vga::Modes::Graphic_320x240_16);
        } break;
        case 12: 
        {
            mode_->switch_to(vga::Modes::Graphic_320x240_12bit);
        } break;
    }
}

void HumanInterface::process(uint8_t byte)
{
    ::write(STDOUT_FILENO, &byte, sizeof(byte));
    if (escape_code_ && byte == 27)
    {
        state_ = State::waiting_for_command;
        position_ = 0;
        return;
    }

    switch (state_)
    {
        case State::waiting_for_command:
        {
            if (byte == '\b')
            {
                if (position_ > 0) position_--;
                buffer_[position_] = 0;
                return;
            }
            if (byte == '\n' || byte == '\r')
            {
                buffer_[position_] = 0;
                position_ = 0;
                process_command();
                return;
            }

            buffer_[position_] = byte;
            if (position_ < sizeof(buffer_) - 1)
            {
                ++position_;
            }
 
        } break; 
        case State::writing:
        {
            process_write(byte);
        } break; 
    }
}

void HumanInterface::process_write(uint8_t byte)
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

void HumanInterface::help() 
{
    const auto arg = get_next_part();

    if (arg.size())
    {
        if (arg == "mode")
        {
            printf("Select mode of display\nAvailable modes:\n");
            printf("  1 - Text mode 80x30 with 16 color pallete\n");
            printf("  2 - Text mode 40x30 with 16 color pallete\n");
            printf("  3 - Text mode 40x30 with 12-bit RGB\n");
            printf("  10 - Graphic mode 640x480 with 16 color pallete\n");
            printf("  11 - Graphic mode 320x240 with 16 color pallete\n");
            printf("  12 - Graphic mode 320x240 with 12-bit RGB\n");
        }

        else
        {
            printf("Not recognized argument for help: ");
            for (const char c : arg)
            {
                printf("%c", c);
            }
            printf("\n");
        }


        return;
    }

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
    printf("To parse: %s (%d)\n", to_parse_.data(), to_parse_.size());
    const auto next_part = to_parse_.substr(0, std::min(to_parse_.find_first_of(" "), to_parse_.size()));
    printf("Next part: %s (%d)\n", next_part.data(), next_part.size());
    to_parse_.remove_prefix(next_part.size());
    return next_part;
}

void HumanInterface::clear()
{
    printf("Clearing screen\n");
    mode_->clear();
}

} // namespace processor
