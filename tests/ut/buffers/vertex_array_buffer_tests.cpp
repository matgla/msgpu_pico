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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "buffers/vertex_array_buffer.hpp"

#include "memory_mock.hpp"

namespace msgpu::buffers
{

class VertexArrayBufferShould : public ::testing::Test
{
  public:
    VertexArrayBufferShould()
        : memory_()
        , sut_(memory_)
    {
    }

  protected:
    ::testing::StrictMock<mocks::MemoryMock> memory_;
    VertexArrayBuffer<mocks::MemoryMock, 128> sut_;
};

TEST_F(VertexArrayBufferShould, AllocateNames)
{
    uint16_t ids[3];

    sut_.allocate_names(3, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 1, 2}));

    sut_.allocate_names(2, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({3, 4, 2}));
}

TEST_F(VertexArrayBufferShould, ReleaseNames)
{
    uint16_t ids[4];

    sut_.allocate_names(4, ids);

    uint16_t to_release[2] = {ids[0], ids[2]};
    sut_.release_names(2, to_release);

    sut_.allocate_names(4, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 2, 4, 5}));
}

TEST_F(VertexArrayBufferShould, WriteDataToBuffer)
{
    VertexArrayBufferData data{
        .size       = 3,
        .normalized = 1,
        .type       = 3,
        .index      = 1,
        .stride     = 123,
    };

    VertexArrayBufferData data2{
        .size       = 9,
        .normalized = 0,
        .type       = 93,
        .index      = 4,
        .stride     = 23,
    };

    EXPECT_FALSE(sut_.set(0, data));
    uint16_t ids[2];
    sut_.allocate_names(2, ids);

    EXPECT_CALL(memory_, write(sut_.start_address, &data, sizeof(VertexArrayBufferData))).Times(1);

    EXPECT_TRUE(sut_.set(ids[0], data));

    EXPECT_CALL(memory_, write(sut_.start_address + sizeof(VertexArrayBufferData), &data2,
                               sizeof(VertexArrayBufferData)))
        .Times(1);

    EXPECT_TRUE(sut_.set(ids[1], data2));
}

} // namespace msgpu::buffers
