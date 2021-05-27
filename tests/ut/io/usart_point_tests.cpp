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

#include <cstring> 

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tests/ut/mocks/arch/hal_dma_mocks.hpp"

#include "io/usart_point.hpp"

#include "messages/header.hpp"

HalDmaMock* hal_dma_mock;

constexpr uint8_t frame_start_byte = 0x7e;

namespace msgpu::io 
{

class UsartPointShould : public testing::Test 
{
protected: 
    UsartPointShould()
        : sut_(data_)
    {
        hal_dma_mock = &hal_dma_mock_;
    }

    void TearDown() override
    {
        EXPECT_TRUE(testing::Mock::VerifyAndClearExpectations(&hal_dma_mock));
        testing::Mock::AllowLeak(&hal_dma_mock);
    }

    void notify_dma_complete()
    {
        sut_.process_event(dma_finished{});  
    }

    UsartPoint data_;
    boost::sml::sm<UsartPoint> sut_;

    HalDmaMock hal_dma_mock_;
};

TEST_F(UsartPointShould, ReceiveCorrectPackage)
{
    void* buffer;
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    sut_.process_event(init{});
    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    std::memcpy(buffer, &frame_start_byte, sizeof(frame_start_byte));
    

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(sizeof(Header), true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    Header header {
        .id = 10,
        .size = 16, 
    };

    std::memcpy(buffer, &header, sizeof(Header));

    EXPECT_CALL(hal_dma_mock_, get_dma_crc());
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));
    notify_dma_complete();

}

TEST_F(UsartPointShould, ReceivePackageWithHeaderOffset)
{
    void* buffer;

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(::testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    sut_.process_event(init{});

    const uint8_t not_start_byte = ~frame_start_byte;
    std::memcpy(buffer, &not_start_byte, sizeof(not_start_byte));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(::testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
  
    notify_dma_complete();

}

} // namespace msgpu::io

