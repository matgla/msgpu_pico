/*
 *   Copyright (c) 2021 Mateusz Stadnik

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <array>
#include <bitset>

#include <msos/dynamic_linker/loaded_module.hpp>

#include "mode/module.hpp"

namespace msgpu::mode
{

constexpr std::size_t MAX_MODULES_LIST_SIZE = 10;

class Program
{
  public:
    Program() = default;
    Program(const std::array<Module, MAX_MODULES_LIST_SIZE> &modules)
        : loaded_modules_(&modules)
    {
    }
    class const_iterator
    {
      public:
        const_iterator(const Program *program, std::size_t index)
            : program_(program)
            , index_(index)
        {
        }

        const Module &operator*() const
        {
            return program_->loaded_modules_->at(index_);
        }

        const Module *operator->() const
        {
            return &program_->loaded_modules_->at(index_);
        }

        const_iterator operator++(int)
        {
            std::size_t prev_index = index_;
            for (; index_ < program_->modules_.size(); ++index_)
            {
                if (program_->modules_.test(index_))
                {
                    return const_iterator(program_, prev_index);
                }
            }
            return const_iterator(program_, prev_index);
        }

        const_iterator &operator++()
        {
            for (; index_ < program_->modules_.size(); ++index_)
            {
                if (program_->modules_.test(index_))
                {
                    return *this;
                }
            }
            return *this;
        }

        bool operator==(const const_iterator &it) const
        {
            return it.index_ == index_;
        }

        bool operator!=(const const_iterator &it) const
        {
            return it.index_ != index_;
        }

      private:
        const Program *program_;
        std::size_t index_;
    };

    bool assign_module(std::size_t module_id)
    {
        if (module_id >= modules_.size())
        {
            return false;
        }
        modules_[module_id] = 1;
        return true;
    }

    const_iterator begin() const
    {
        for (std::size_t index = 0; index < modules_.size(); ++index)
        {
            if (modules_.test(index))
            {
                return const_iterator(this, index);
            }
        }
        return end();
    }

    const_iterator end() const
    {
        for (std::size_t index = modules_.size() - 1; index > 0; --index)
        {
            if (modules_.test(index))
            {
                return const_iterator(this, index);
            }
        }
        return const_iterator(this, -1);
    }

  private:
    std::bitset<MAX_MODULES_LIST_SIZE> modules_;
    const std::array<Module, MAX_MODULES_LIST_SIZE> *loaded_modules_;
};

} // namespace msgpu::mode