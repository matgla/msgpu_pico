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

#include <cstring>

#include "processor/message_processor.hpp"

namespace msgpu::processor 
{
struct A
{
    constexpr static uint32_t id = 1;
    int a;
};

struct B 
{
    constexpr static uint32_t id = 3;
    int a; 
    int b;
};

bool operator==(const B& a, const B& b)
{
    return a.a == b.a && a.b == b.b;
}

bool operator==(const A& a, const A& b)
{
    return a.a == b.a;
}


struct AHandler
{
    MOCK_METHOD1(process_a, bool(const A& a));
};

struct BHandler
{
    MOCK_METHOD1(process_b, bool(const B& a));
};

TEST(MessageProcessorShould, RegisterMessages)
{
    MessageProcessor sut;
    AHandler mock_a;
    BHandler mock_b;

    sut.register_handler<A>(&AHandler::process_a, &mock_a);
    sut.register_handler<B>(&BHandler::process_b, &mock_b);

    A a = { .a = 124 };
    B b = { .a = 10, .b = 20 };

    EXPECT_CALL(mock_a, process_a(a));
    EXPECT_CALL(mock_b, process_b(b));

    io::Message msg_a {
        .received = true,
        .header = {
            .id = A::id,
            .size = sizeof(A)
        },
        .payload = {}
    };

    std::memcpy(msg_a.payload.data(), &a, sizeof(A));

    io::Message msg_b {
        .received = true,
        .header = {
            .id = B::id,
            .size = sizeof(B)
        },
        .payload = {}
    };

    std::memcpy(msg_b.payload.data(), &b, sizeof(B));

    sut.process_message(msg_a);
    sut.process_message(msg_b);

    io::Message msg_c {
        .received = true,
        .header = {
            .id = 10,
            .size = 0
        },
        .payload = {}
    };

    sut.process_message(msg_c);
}

} // namespace msgpu::processor

