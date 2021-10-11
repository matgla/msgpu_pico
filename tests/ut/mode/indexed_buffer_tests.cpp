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

#include "mode/indexed_buffer.hpp"

#include <gtest/gtest.h>

namespace msgpu::mode
{

TEST(IndexedBufferShould, AllocateSlot)
{
    constexpr std::size_t size = 5;
    IndexedBuffer<int, size> sut;

    for (std::size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(i, sut.allocate());
    }
    EXPECT_EQ(-1, sut.allocate());
}

TEST(IndexedBufferShould, DeallocateSlot)
{
    constexpr std::size_t size = 5;
    IndexedBuffer<int, size> sut;

    for (std::size_t i = 0; i < size; ++i)
    {
        sut.allocate();
    }

    sut.deallocate(2);
    EXPECT_EQ(sut.allocate(), 2);
    sut.deallocate(size + 1);
    EXPECT_EQ(sut.allocate(), -1);
}

TEST(IndexedBufferShould, ReturnsFalseIfIdNotAllocated)
{
    constexpr std::size_t size = 2;
    IndexedBuffer<int, size> sut;

    EXPECT_FALSE(sut.write(2, 10));
}

TEST(IndexedBufferShould, WriteData)
{
    constexpr std::size_t size = 2;
    IndexedBuffer<int, size> sut;

    const std::size_t id_1 = sut.allocate();
    const std::size_t id_2 = sut.allocate();

    sut.write(id_1, 1);
    sut.write(id_2, 4);

    EXPECT_EQ(sut[id_1], 1);
    const int &val2 = sut[id_2];
    EXPECT_EQ(val2, 4);

    int &val1 = sut[id_1];
    val1      = 15;

    EXPECT_EQ(sut[id_1], 15);

    sut[id_2] = 10;
    EXPECT_EQ(sut[id_2], 10);
}

TEST(IndexedBufferShould, TestUsage)
{
    constexpr std::size_t size = 2;
    IndexedBuffer<int, size> sut;

    for (std::size_t i = 0; i < size; ++i)
    {
        EXPECT_FALSE(sut.test(i));
    }
    sut.allocate();
    EXPECT_TRUE(sut.test(0));
    EXPECT_FALSE(sut.test(1));
}

} // namespace msgpu::mode
