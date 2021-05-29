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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mode/modes.hpp"

namespace msgpu::mode 
{

struct A 
{
    int a;
    int b;

    bool operator==(const A& other) const
    {
        return other.a == a && other.b == b;
    }
};

struct B 
{
    const char* text;

    bool operator==(const B& other) const 
    {
        return std::string_view(text) == std::string_view(other.text);
    }
};

struct C
{
    bool operator==(const C& ) const 
    {
        return true;
    }
};

struct HandlerAMock 
{
    MOCK_METHOD1(process, void(const A& a));
    MOCK_METHOD1(process, void(const C& c));
};


struct HandlerBMock 
{
    MOCK_METHOD1(process, void(const B& b));
    MOCK_METHOD1(process, void(const A& c));
};


struct HandlerAWrap
{
    HandlerAWrap(::testing::StrictMock<HandlerAMock>* mock) : mock_(mock) {}

    void process(const A& a)
    {
        mock_->process(a);
    }
 
    void process(const C& a)
    {
        mock_->process(a);
    }


    ::testing::StrictMock<HandlerAMock>* mock_;
};

struct HandlerBWrap
{
    HandlerBWrap(::testing::StrictMock<HandlerBMock>* mock) : mock_(mock) {}

    void process(const B& b)
    {
        mock_->process(b);
    }
 
    void process(const A& a)
    {
        mock_->process(a);
    }


    ::testing::StrictMock<HandlerBMock>* mock_;
};



TEST(ModesShould, RegisterModes)
{
    ::testing::StrictMock<HandlerAMock> mocka;
    ::testing::StrictMock<HandlerBMock> mockb;

    using MockA = HandlerAWrap;
    using MockB = HandlerBWrap;

    auto sut = ModesFactory{}
        .add_mode<MockA>()
        .add_mode<MockB>()
        .create();

    sut.switch_to<MockA>(&mocka);

    A a {
        .a = 1,
        .b = 2
    };

    EXPECT_CALL(mocka, process(a));
    sut.process(a);
    sut.process(B{"Hello"});
    C c {};
    EXPECT_CALL(mocka, process(c));
    sut.process(c);
 
    sut.switch_to<MockB>(&mockb);

    A a2 {
        .a = 3, 
        .b = 4
    };

    B b {
        .text = "Hello"
    };

    EXPECT_CALL(mockb, process(a2));
    sut.process(a2);

    EXPECT_CALL(mockb, process(b));
    sut.process(b);
    sut.process(C{});
}

} // namespace msgpu::mode
