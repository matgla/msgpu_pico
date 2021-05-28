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

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc = 0x1234;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc, sizeof(some_crc));
    // After CRC there will be header validation, and setup 

    const uint8_t data[] = {1, 2, 3, 4, 5, 6, 10, 100, 15, 12, 255, 0x7e, 0x7e, 10, 40, 50};

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(header.size, true));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    // Send payload, expect to fetch payload CRC
    std::memcpy(buffer, &data, header.size);
    const uint16_t payload_crc = 0xabcd; 
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(payload_crc));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));

    notify_dma_complete();

    std::memcpy(buffer, &payload_crc, sizeof(payload_crc));

    // Now send second message to verify queue 

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
   
    notify_dma_complete();
    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    std::memcpy(buffer, &frame_start_byte, sizeof(frame_start_byte));
    

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(sizeof(Header), true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();


    Header header2 {
        .id = 2,
        .size = 8, 
    };

    std::memcpy(buffer, &header2, sizeof(Header));

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc2 = 0x4321;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc2));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc2, sizeof(some_crc2));
    // After CRC there will be header validation, and setup 

    const uint8_t data2[] = {0xd, 0xe, 0xa, 0xd, 0xb, 0xe, 0x7e, 0xf};

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(header2.size, true));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    // Send payload, expect to fetch payload CRC
    std::memcpy(buffer, &data2, header2.size);
    const uint16_t payload_crc2 = 0xface; 
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(payload_crc2));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));

    notify_dma_complete();

    std::memcpy(buffer, &payload_crc2, sizeof(payload_crc));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));

    notify_dma_complete();

    // And now we are ready to check received payload 
    std::array<uint8_t, 32> expected_payload_1{};
    std::copy(std::begin(data), std::end(data), expected_payload_1.begin());
    const auto msg1 = data_.pop();
    EXPECT_TRUE(msg1);
    EXPECT_EQ(msg1->header.id, header.id);
    EXPECT_EQ(msg1->header.size, header.size);
    EXPECT_THAT(msg1->payload, ::testing::ElementsAreArray(expected_payload_1));

    std::array<uint8_t, 32> expected_payload_2{};
    std::copy(std::begin(data2), std::end(data2), expected_payload_2.begin());
    const auto msg2 = data_.pop(); 
    EXPECT_TRUE(msg2);
    EXPECT_EQ(msg2->header.id, header2.id);
    EXPECT_EQ(msg2->header.size, header2.size);
    EXPECT_THAT(msg2->payload, ::testing::ElementsAreArray(expected_payload_2));

    const auto msg3 = data_.pop();
    EXPECT_FALSE(msg3);

}

TEST_F(UsartPointShould, ReceivePackageWithHeaderOffset)
{
    void* buffer;
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    sut_.process_event(init{});
    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    constexpr uint16_t not_start_byte = ~frame_start_byte;
    std::memcpy(buffer, &not_start_byte, sizeof(not_start_byte));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    notify_dma_complete();

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

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc = 0x1234;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc, sizeof(some_crc));
    // After CRC there will be header validation, and setup 

    const uint8_t data[] = {1, 2, 3, 4, 5, 6, 10, 100, 15, 12, 255, 0x7e, 0x7e, 10, 40, 50};

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(header.size, true));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    // Send payload, expect to fetch payload CRC
    std::memcpy(buffer, &data, header.size);
    const uint16_t payload_crc = 0xabcd; 
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(payload_crc));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));

    notify_dma_complete();

    std::memcpy(buffer, &payload_crc, sizeof(payload_crc));

    // Now send second message to verify queue 

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
   
    notify_dma_complete();

    std::memcpy(buffer, &not_start_byte, sizeof(not_start_byte));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    notify_dma_complete();


    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    std::memcpy(buffer, &frame_start_byte, sizeof(frame_start_byte));
    

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(sizeof(Header), true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();


    Header header2 {
        .id = 2,
        .size = 8, 
    };

    std::memcpy(buffer, &header2, sizeof(Header));

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc2 = 0x4321;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc2));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc2, sizeof(some_crc2));
    // After CRC there will be header validation, and setup 

    const uint8_t data2[] = {0xd, 0xe, 0xa, 0xd, 0xb, 0xe, 0x7e, 0xf};

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(header2.size, true));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    // Send payload, expect to fetch payload CRC
    std::memcpy(buffer, &data2, header2.size);
    const uint16_t payload_crc2 = 0xface; 
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(payload_crc2));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(::testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));

    notify_dma_complete();

    std::memcpy(buffer, &payload_crc2, sizeof(payload_crc));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));

    notify_dma_complete();

    // And now we are ready to check received payload 
    std::array<uint8_t, 32> expected_payload_1{};
    std::copy(std::begin(data), std::end(data), expected_payload_1.begin());
    const auto msg1 = data_.pop();
    EXPECT_TRUE(msg1);
    EXPECT_EQ(msg1->header.id, header.id);
    EXPECT_EQ(msg1->header.size, header.size);
    EXPECT_THAT(msg1->payload, ::testing::ElementsAreArray(expected_payload_1));

    std::array<uint8_t, 32> expected_payload_2{};
    std::copy(std::begin(data2), std::end(data2), expected_payload_2.begin());
    const auto msg2 = data_.pop(); 
    EXPECT_TRUE(msg2);
    EXPECT_EQ(msg2->header.id, header2.id);
    EXPECT_EQ(msg2->header.size, header2.size);
    EXPECT_THAT(msg2->payload, ::testing::ElementsAreArray(expected_payload_2));

    const auto msg3 = data_.pop();
    EXPECT_FALSE(msg3);
}

TEST_F(UsartPointShould, ReceiveMessageWithZeroSize)
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
        .size = 0, 
    };

    std::memcpy(buffer, &header, sizeof(Header));

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc = 0x1234;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc, sizeof(some_crc));
    // After CRC there will be header validation, and setup 


    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
   
    notify_dma_complete();


    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    std::memcpy(buffer, &frame_start_byte, sizeof(frame_start_byte));
    

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(sizeof(Header), true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    Header header2 {
        .id = 2,
        .size = 0, 
    };

    std::memcpy(buffer, &header2, sizeof(Header));

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc2 = 0x4321;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc2));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc2, sizeof(some_crc2));
    // After CRC there will be header validation, and setup 

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));

    notify_dma_complete();

    // And now we are ready to check received payload 
    std::array<uint8_t, 32> expected_payload_1{};
    const auto msg1 = data_.pop();
    EXPECT_TRUE(msg1);
    EXPECT_EQ(msg1->header.id, header.id);
    EXPECT_EQ(msg1->header.size, header.size);
    EXPECT_THAT(msg1->payload, ::testing::ElementsAreArray(expected_payload_1));

    std::array<uint8_t, 32> expected_payload_2{};
    const auto msg2 = data_.pop(); 
    EXPECT_TRUE(msg2);
    EXPECT_EQ(msg2->header.id, header2.id);
    EXPECT_EQ(msg2->header.size, header2.size);
    EXPECT_THAT(msg2->payload, ::testing::ElementsAreArray(expected_payload_2));

    const auto msg3 = data_.pop();
    EXPECT_FALSE(msg3);
}

TEST_F(UsartPointShould, DropFramesWithWrongSize)
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
        .size = 33, 
    };

    std::memcpy(buffer, &header, sizeof(Header));

    // Get CRC for message, for now it may be some value
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(testing::Return(0));

    notify_dma_complete();

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));

    notify_dma_complete();

    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    std::memcpy(buffer, &frame_start_byte, sizeof(frame_start_byte));
    

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(sizeof(Header), true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    Header header2 {
        .id = 2,
        .size = 0, 
    };

    std::memcpy(buffer, &header2, sizeof(Header));

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc2 = 0x4321;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc2));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc2, sizeof(some_crc2));
    // After CRC there will be header validation, and setup 

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));

    notify_dma_complete();

    // And now we are ready to check received payload 
    std::array<uint8_t, 32> expected_payload_1{};
    const auto msg1 = data_.pop();
    EXPECT_TRUE(msg1);
    EXPECT_EQ(msg1->header.id, header2.id);
    EXPECT_EQ(msg1->header.size, header2.size);
    EXPECT_THAT(msg1->payload, ::testing::ElementsAreArray(expected_payload_1));

    const auto msg2 = data_.pop();
    EXPECT_FALSE(msg2);
}

TEST_F(UsartPointShould, DropFramesWithWrongHeaderCrc)
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
        .size = 2, 
    };

    std::memcpy(buffer, &header, sizeof(Header));

    // Get CRC for message, for now it may be some value
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(testing::Return(0));

    notify_dma_complete();

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    
    const uint16_t some_crc = 0x1234;
    std::memcpy(buffer, &some_crc, sizeof(some_crc));
    notify_dma_complete();

    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    std::memcpy(buffer, &frame_start_byte, sizeof(frame_start_byte));
    

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(sizeof(Header), true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    Header header2 {
        .id = 2,
        .size = 0, 
    };

    std::memcpy(buffer, &header2, sizeof(Header));

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc2 = 0x4321;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc2));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc2, sizeof(some_crc2));
    // After CRC there will be header validation, and setup 

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));

    notify_dma_complete();

    // And now we are ready to check received payload 
    std::array<uint8_t, 32> expected_payload_1{};
    const auto msg1 = data_.pop();
    EXPECT_TRUE(msg1);
    EXPECT_EQ(msg1->header.id, header2.id);
    EXPECT_EQ(msg1->header.size, header2.size);
    EXPECT_THAT(msg1->payload, ::testing::ElementsAreArray(expected_payload_1));

    const auto msg2 = data_.pop();
    EXPECT_FALSE(msg2);
}

TEST_F(UsartPointShould, DropFramesWithWrongPayloadCrc)
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
        .size = 2, 
    };

    std::memcpy(buffer, &header, sizeof(Header));

    const uint16_t some_crc = 0x1234;
    // Get CRC for message, for now it may be some value
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(testing::Return(some_crc));

    notify_dma_complete();

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(header.size, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());
    std::memcpy(buffer, &some_crc, sizeof(some_crc));
    
    notify_dma_complete();
  

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer)); 
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(testing::Return(0xabcd));
   
    const uint8_t data[] = {1, 2};

    std::memcpy(buffer, data, header.size);

    notify_dma_complete();

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));

    const uint16_t wrong_crc = 0x1234;
    std::memcpy(buffer, &wrong_crc, sizeof(wrong_crc));

    notify_dma_complete();

    // Now UsartPoint is waiting for 0x7e symbol which starts frame
    // buffer now is pointing to 1 byte element 
    std::memcpy(buffer, &frame_start_byte, sizeof(frame_start_byte));
    

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(sizeof(Header), true));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, reset_dma_crc());

    notify_dma_complete();

    Header header2 {
        .id = 2,
        .size = 0, 
    };

    std::memcpy(buffer, &header2, sizeof(Header));

    // Get CRC for message, for now it may be some value
    const uint16_t some_crc2 = 0x4321;
    EXPECT_CALL(hal_dma_mock_, get_dma_crc())
        .WillOnce(::testing::Return(some_crc2));

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false))
        .WillOnce(testing::SaveArg<0>(&buffer));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(2, true));


    notify_dma_complete();

    std::memcpy(buffer, &some_crc2, sizeof(some_crc2));
    // After CRC there will be header validation, and setup 

    EXPECT_CALL(hal_dma_mock_, set_usart_dma_buffer(testing::_, false));
    EXPECT_CALL(hal_dma_mock_, set_usart_dma_transfer_count(1, true));

    notify_dma_complete();

    // And now we are ready to check received payload 
    std::array<uint8_t, 32> expected_payload_1{};
    const auto msg1 = data_.pop();
    EXPECT_TRUE(msg1);
    EXPECT_EQ(msg1->header.id, header2.id);
    EXPECT_EQ(msg1->header.size, header2.size);
    EXPECT_THAT(msg1->payload, ::testing::ElementsAreArray(expected_payload_1));

    const auto msg2 = data_.pop();
    EXPECT_FALSE(msg2);
}



} // namespace msgpu::io


