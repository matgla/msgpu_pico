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

#pragma once 

#include <array>
#include <cstdint>
#include <optional>

#include <boost/sml.hpp>

#include <eul/container/static_deque.hpp>

#include "io/message.hpp"

namespace msgpu::io 
{

struct init{};
struct dma_finished{};

/// @brief Implements external world interface with standard USART protocol
/// 
/// @details 
///  Simple class to receive messages from external devices via USART protocol. 
///  Received data is validated, but retransmission of frames is not supported.
///  Performance is more desirable than reliability. 
///  Frames are limited to 32 bytes.
///  Protocol uses little endian byte order. 
///  Frames format: 
///  +--------------+
///  |     0x7e     |  1 - byte
///  +--------------+
///  |      id      |  2 - byte 
///  +--------------+  
///  |     size     |  3 - byte 
///  +--------------+ 
///  |              |  4 - byte  
///  +  CRC16-CCIT  + 
///  |              |  5 - byte 
///  +--------------+
///  |              |
///  .    payload   .  0 - 32 bytes
///  |              |
///  +--------------+
///  |              |  
///  +  CRC16-CCIT  +  0 - 2 bytes  
///  |              |   
///  +--------------+ 
///
/// @author Mateusz Stadnik
class UsartPoint
{
    using Self = UsartPoint;
public:
    auto operator()() 
    {
        using namespace boost::sml;

        auto const got_frame_start = wrap(&Self::got_start_token);
        auto const verify = wrap(&Self::verify_crc);
        auto const empty_message = wrap(&Self::message_without_size);
        auto const verify_header = wrap(&Self::check_header);
        return make_transition_table(
           *"init"_s                 + event<init>                                    
                        / (&Self::prepare_for_token)  = "wait_for_start_token"_s,
            "wait_for_start_token"_s + event<dma_finished> [ got_frame_start ] 
                        / (&Self::prepare_for_header) = "wait_for_header"_s, 
            "wait_for_start_token"_s + event<dma_finished> [ !got_frame_start ] 
                        / (&Self::prepare_for_token)  = "wait_for_start_token"_s,
            "wait_for_header"_s + event<dma_finished>
                        / (&Self::prepare_for_crc)    = "wait_for_header_crc"_s,
            "wait_for_header_crc"_s + event<dma_finished> [ verify && verify_header && !empty_message ]
                        / (&Self::prepare_for_payload) = "wait_for_payload"_s,
            "wait_for_header_crc"_s + event<dma_finished> [ verify && empty_message ] 
                        / (wrap(&Self::store_message), &Self::prepare_for_token) = "wait_for_start_token"_s,
            "wait_for_header_crc"_s + event<dma_finished> [ !verify || !verify_header] 
                        / (wrap(&Self::drop_message), &Self::prepare_for_token)  = "wait_for_start_token"_s, 
            "wait_for_payload"_s + event<dma_finished> 
                        / (&Self::prepare_for_crc)   = "wait_for_payload_crc"_s, 
            "wait_for_payload_crc"_s + event<dma_finished> [ verify ] 
                        / (wrap(&Self::store_message), &Self::prepare_for_token) = "wait_for_start_token"_s, 
            "wait_for_payload_crc"_s + event<dma_finished> [ !verify ] 
                        / (wrap(&Self::drop_message), &Self::prepare_for_token) = "wait_for_start_token"_s

        );
    }

    /// @brief Provide access to ready messages 
        /// @return Message object if it's ready to process or none if queue is empty
    std::optional<Message> pop();
private:
    // ==================== GUARDS ==================//
    /// @brief Checks if token buffer contains 0x7e, which is symbol for frame start.
    bool got_start_token();
    /// @brief Checks if CRC received from peer is same as calculated one.
    bool verify_crc();
    /// @brief Checks if message contains only header (id, without any payload).
    bool message_without_size();

    /// @brief Checks if payload size is correct (<=32 bytes) 
    bool check_header();

    // =================== ACTIONS ==================//

    /// @brief Setup DMA controller to fetch header from usart. 
    /// 
    /// @details 
    ///  This function also resets DMA CRC block to calculate CRC via hardware.
    ///  Used algorithm is CRC16-CCIT with 0x0000 initial state.
    ///  Header will be stored in \ref UsartPoint::current_message_.header.
    ///
    void prepare_for_header();

    /// @brief Setup DMA controller to fetch single byte. 
    /// 
    /// @details 
    ///  This member functions is used to synchronize frame in case of transmission errors. 
    ///  Start frame token will be stored in \ref UsartPoint::token_buffer_.
    ///
    void prepare_for_token();

    /// @brief Setup DMA controller to fetch payload. 
    /// 
    /// @details 
    ///  This function resets DMA CRC block to correctly calculate CRC for payload. 
    ///  Used algorithm is CRC16-CCIT with 0x0000 initial state. 
    ///  
    ///  Size of payload is taken from last captured message header \ref UsartPoint::current_message_.header.
    ///  Payload will be stored in \ref UsartPoint::current_message_.payload
    ///
    void prepare_for_payload();

    /// @brief Setup DMA controller to fetch 2-byte CRC from peer.  
    /// 
    /// @details 
    ///   Used to capture CRC value calculated by peer for header and payload. 
    ///   CRC is stored in \ref UsartPoint::received_crc_.
    ///
    void prepare_for_crc();

    /// @brief Store received message in message queue 
    /// 
    /// @details 
    ///   Used to store message in queue for further processing. 
    ///   Data in buffer contains header and 32 bytes of raw bytes. 
    ///   Further processing by be done by other component 
    /// 
    void store_message();

    /// @brief Drop message from buffer in case of failure.
    void drop_message();

    uint8_t token_buffer_;
    uint16_t got_crc_;
    uint16_t received_crc_;
    Message* current_message_;
    eul::container::static_deque<Message, 32> messages_;
};

} // namespace msgpu::io 

