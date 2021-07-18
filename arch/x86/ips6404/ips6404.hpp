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
#include <boost/sml.hpp>

namespace msgpu 
{
namespace stubs 
{

class IPS6404Stub : public msgpu::IDevice
{
private: 
    using Self = IPS6404Stub;
public:
    struct evInit{};
    struct evWrite{};
    struct evRead{};
    struct evTransmit{};

    IPS6404Stub(std::string_view name);
    ~IPS6404Stub();
    void init() override;
    void read(const DataType& buf, std::size_t len) override;
    void write(const ConstDataType& buf, std::size_t len) override;
    void transmit(const ConstDataType& buf, const DataType& dest, std::size_t write_len, std::size_t read_len) override;

    auto operator()()
    {
        using namespace boost::sml;
        
        return make_transition_table(
            *"init"_s            + event<Self::evInit>  / (&Self::init)        = "wait_for_command"_s,
            "wait_for_command"_s + event<Self::evWrite> / (&Self::get_command) = "receive_data"_s 
        );
    }
private: 
    void get_command();

    int memory_fd_;
    std::string_view buffer_name_;

};

} // namespace stubs
} // namespace msgpu
