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

#include "buffers/gpu_buffers.hpp"

#include "memory_mock.hpp"

namespace msgpu::buffers
{

class GpuBuffersShould : public ::testing::Test
{
  public:
    GpuBuffersShould()
        : memory_()
        , sut_(memory_)
    {
    }

  protected:
    mocks::MemoryMock memory_;
    GpuBuffers<mocks::MemoryMock> sut_;
};

TEST_F(GpuBuffersShould, AllocateNames)
{
    uint16_t ids[3];

    sut_.allocate_names(3, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 1, 2}));

    sut_.allocate_names(2, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({3, 4, 2}));
}

TEST_F(GpuBuffersShould, ReleaseNames)
{
    uint16_t ids[4];

    sut_.allocate_names(4, ids);

    uint16_t to_release[2] = {ids[0], ids[2]};
    sut_.release_names(2, to_release);

    sut_.allocate_names(4, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 2, 4, 5}));
}

TEST_F(GpuBuffersShould, AllocateMemoryWithCorrectSlots)
{
    constexpr std::size_t block_size = 1024;

    uint16_t id;
    uint16_t id2;
    uint16_t id3;

    uint8_t data[block_size * 2 + 1] = {};
    uint8_t data2[block_size * 2];
    uint8_t data3[block_size];
    {
        sut_.allocate_names(1, &id);
        // This allocs segments 0, 1, 2
        EXPECT_CALL(memory_, write(0x0, data, sizeof(data)));
        sut_.allocate_memory(id, sizeof(data));
        sut_.write(id, data, sizeof(data));

        sut_.allocate_names(1, &id2);
        // This will alloc segments 3, 4
        EXPECT_CALL(memory_, write(block_size * 3, data2, sizeof(data2)));
        sut_.allocate_memory(id2, sizeof(data2));
        sut_.write(id2, data2, sizeof(data2));

        // Release 0, 1, 2
        sut_.release_names(1, &id);
        sut_.allocate_names(1, &id);
        sut_.allocate_names(1, &id3);

        // Alloc 0
        EXPECT_CALL(memory_, write(0x0, data3, sizeof(data3)));
        sut_.allocate_memory(id3, sizeof(data3));
        sut_.write(id3, data3, sizeof(data3));
    }
    {
        // Alloc 1, 2
        EXPECT_CALL(memory_, write(block_size, data2, sizeof(data2)));
        sut_.allocate_memory(id, sizeof(data2));
        sut_.write(id, data2, sizeof(data2));

        // Dealloc 1
        sut_.release_names(1, &id3);

        // Reuse 3, 4
        EXPECT_CALL(memory_, write(block_size * 3, data2, sizeof(data2)));
        sut_.allocate_memory(id2, sizeof(data2));
        sut_.write(id2, data2, sizeof(data2));

        // Alloc 0
        EXPECT_CALL(memory_, write(0x0, data3, sizeof(data3)));
        sut_.allocate_names(1, &id3);
        sut_.allocate_memory(id3, sizeof(data3));
        sut_.write(id3, data3, sizeof(data3));
    }
    // Reloc 1, 2 to 5, 6
    EXPECT_CALL(memory_, write(block_size * 5, data, sizeof(data)));
    sut_.allocate_memory(id, sizeof(data));
    sut_.write(id, data, sizeof(data));
}

} // namespace msgpu::buffers
