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

#include "buffers/id_generator.hpp"

#include "memory_mock.hpp"

namespace msgpu::buffers
{

class IdGeneratorShould : public ::testing::Test
{
  public:
    IdGeneratorShould()
        : sut_{}
    {
    }

  protected:
    IdGenerator<1024> sut_;
};

TEST_F(IdGeneratorShould, AllocateNames)
{
    uint16_t ids[3];

    ids = {
        sut_.allocate_name(),
        sut_.allocate_name(),
        sut_.allocate_name(),
    };
    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 1, 2}));

    ids = {
        sut_.allocate_name(),
        sut_.allocate_name(),
    };
    EXPECT_THAT(ids, ::testing::ElementsAreArray({3, 4, 2}));
}

TEST_F(GpuBuffersShould, ReleaseNames)
{
    uint16_t ids[4];

    ids = {
        sut_.allocate_name(),
        sut_.allocate_name(),
        sut_.allocate_name(),
        sut_.allocate_name(),
    };

    sut_.release_name(ids[0]);
    sut_.release_name(ids[2]);

    ids = {
        sut_.allocate_name(),
        sut_.allocate_name(),
        sut_.allocate_name(),
        sut_.allocate_name(),
    };

    EXPECT_THAT(ids, ::testing::ElementsAreArray({0, 2, 4, 5}));
}

} // namespace msgpu::buffers
