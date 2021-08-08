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

#include "qspi_bus.hpp"

#include <string_view>

#include <semaphore.h>

#include <boost/sml.hpp>

namespace msgpu 
{
namespace stubs 
{

constexpr static std::size_t memory_size = 8 * 1024 * 1024;

struct SharedMemory 
{
    int fd;
    std::string_view name;
    sem_t* sem;
    std::span<uint8_t> memory;
};



class IPS6404StubSm 
{
private:
    using Self = IPS6404StubSm;
public: 
    IPS6404StubSm(SharedMemory& memory);

    struct evInit{};
    struct evTransmit{
        const std::span<const uint8_t>& src;
        const std::span<uint8_t>& dest;
        std::size_t src_len;
        std::size_t dest_len;
    };

    auto operator()()
    {
        using namespace boost::sml;

        const auto is_read = [](const evTransmit& ev)
        {
            return ev.src[0] == 0x03;
        };

        const auto is_fast_read = [](const evTransmit& ev)
        {
            return ev.src[0] == 0x0b;
        };

        const auto is_fast_read_quad = [](const evTransmit& ev)
        {
            return ev.src[0] == 0xeb;
        };

        const auto is_write = [](const evTransmit& ev)
        {
            return ev.src[0] == 0x02;
        };

        const auto is_quad_write = [](const evTransmit& ev)
        {
            return ev.src[0] == 0x38;
        };

        const auto is_enter_quad_mode = [](const evTransmit& ev)
        {
            return ev.src[0] == 0x35;
        };

        const auto is_exit_quad_mode = [](const evTransmit& ev)
        {
            return ev.src[0] == 0xf5;
        };
        
        const auto is_reset_enable = [](const evTransmit& ev)
        {
            return ev.src[0] == 0x66;
        };

        const auto is_reset = [](const evTransmit& ev)
        {
            return ev.src[0] == 0x99;
        };

        const auto is_burst_mode_toggle = [](const evTransmit& ev) 
        {
            return ev.src[0] == 0xc0;
        };

        const auto is_read_id = [](const evTransmit& ev) 
        {
            return ev.src[0] == 0x9f;
        };

        const auto is_quad = wrap(&Self::is_quad_mode);
        const auto is_reset_enabled = wrap(&Self::is_reset_enabled);

        return make_transition_table(
            *"init"_s            + event<Self::evInit>  
                    / (&Self::init)  = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit> 
                [ (is_read) && !is_quad ] 
                    / (&Self::read)  = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ is_fast_read && !is_quad ] 
                    / (&Self::spi_fast_read) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ is_fast_read_quad ] 
                    / (&Self::qpi_fast_read) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ (is_write || is_quad_write) && !is_quad ]
                    / (&Self::write) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ (is_write || is_quad_write) && is_quad ]
                    / (&Self::write) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit> 
                [ is_enter_quad_mode && !is_quad ] 
                    / (&Self::switch_to_quad) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ is_exit_quad_mode && is_quad ] 
                    / (&Self::switch_to_spi) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit> 
                [ is_reset_enable ] 
                    / (&Self::enable_reset) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ is_reset && is_reset_enabled ] 
                    / (&Self::reset) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ is_burst_mode_toggle ]
                    / (&Self::burst_mode_toggle) = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evTransmit>
                [ is_read_id ]
                    / (&Self::read_eid) = "wait_for_command"_s
        );
    }

private: 
    void init();
    void read_base(const evTransmit& ev, int wait_cycles);
    void read(const evTransmit& ev);
    void spi_fast_read(const evTransmit& ev);
    void qpi_fast_read(const evTransmit& ev);
    void write(const evTransmit& ev);
    void switch_to_quad();
    void switch_to_spi();
    void enable_reset();
    void reset();
    void burst_mode_toggle(const evTransmit& ev);
    void read_eid(const evTransmit& ev);

    bool is_quad_mode() const;
    bool is_reset_enabled() const;

    SharedMemory& memory_;
    bool quad_mode_ {false};
    bool burst_ {false};
    bool reset_enabled_ {false};
};

class IPS6404Stub : public msgpu::IDevice
{
private: 
    using Self = IPS6404Stub;
public:
    IPS6404Stub(std::string_view name);
    ~IPS6404Stub();
    void init() override;
    void read(const DataType& buf, std::size_t len) override;
    void write(const ConstDataType& buf, std::size_t len) override;
    void transmit(const ConstDataType& buf, const DataType& dest, std::size_t write_len, std::size_t read_len) override;

private: 
    void get_command();

    int memory_fd_;
    std::string_view name_;

    SharedMemory memory_;
    IPS6404StubSm sm_data_;
    boost::sml::sm<IPS6404StubSm> sm_;
};

} // namespace stubs
} // namespace msgpu
