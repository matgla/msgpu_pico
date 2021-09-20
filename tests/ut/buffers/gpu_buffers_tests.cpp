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
    uint32_t ids[3];

    sut_.allocate_names(3, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 1, 2}));

    sut_.allocate_names(2, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({3, 4, 2}));
}

TEST_F(GpuBuffersShould, ReleaseNames)
{
    uint32_t ids[4];

    sut_.allocate_names(4, ids);

    uint32_t to_release[2] = {ids[0], ids[2]};
    sut_.release_names(2, to_release);

    sut_.allocate_names(4, ids);
    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 2, 4, 5}));
}

TEST_F(GpuBuffersShould, AllocateMemoryWithCorrectSlots)
{
    uint32_t id;
    sut_.allocate_names(1, &id);
    constexpr std::size_t block_size = 1024;
    uint8_t data[block_size * 2 + 1] = {1, 2, 3};

    EXPECT_CALL(memory_.write(0x0, data, sizeof(block_size)));
    sut_.write(id, data, sizeof(data));
}

} // namespace msgpu::buffers