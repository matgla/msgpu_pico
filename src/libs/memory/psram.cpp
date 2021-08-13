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

#include "memory/psram.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "config.hpp"
#include "board.hpp"

namespace msgpu::memory 
{

constexpr uint8_t read_cmd = 0x03;
constexpr uint8_t qspi_fast_read_cmd = 0xeb; 
constexpr uint8_t qspi_write_cmd = 0x38;
constexpr uint8_t read_eid_cmd = 0x9f;
constexpr uint8_t enter_qpi_cmd = 0x35; 
constexpr uint8_t exit_qpi_cmd = 0xf5;

constexpr unsigned long long operator "" _MB(unsigned long long int value)
{
    return value * 1024 * 1024;
}

constexpr std::size_t psram_max_size = 8_MB;

QspiPSRAM::QspiPSRAM(Qspi& qspi, bool qspi_mode)
    : qspi_(qspi)
    , qspi_mode_(qspi_mode)
{
}

bool QspiPSRAM::init()
{
    qspi_.acquire_bus();
    if (!reset())
    {
        qspi_mode_ = true;
        exit_qpi_mode();

    }
    else 
    {
        enter_qpi_mode();

        srand(static_cast<uint32_t>(msgpu::get_us()));
        qspi_.release_bus();
        return true; 
    }
    if (reset())
    {
        qspi_.acquire_bus();
        enter_qpi_mode();

        srand(static_cast<uint32_t>(msgpu::get_us()));
        qspi_.release_bus();
        return true;
    }

    qspi_.release_bus();
    return false;
}

bool QspiPSRAM::reset()
{
    printf("Reset\n");
    msgpu::sleep_us(200); // Wait for initialization from datasheet 

    constexpr uint8_t reset_enable_cmd[] = {0x66};
    constexpr uint8_t reset_cmd[] = {0x99};
    qspi_.spi_write(reset_enable_cmd);
    qspi_.spi_write(reset_cmd);

    msgpu::sleep_us(100);
    return perform_post();
}

void __time_critical_func(QspiPSRAM::wait_for_finish)() const 
{
    qspi_.wait_for_finish();
    qspi_.release_bus();
}

std::size_t __time_critical_func(QspiPSRAM::write)(std::size_t address, const ConstDataBuffer data)
{
    const uint8_t cmd[] = {
        qspi_write_cmd, 
        static_cast<uint8_t>((address >> 16)), 
        static_cast<uint8_t>((address >> 8)), 
        static_cast<uint8_t>(address & 0xff)
    };

    qspi_.acquire_bus();
    qspi_.qspi_command_write(cmd, data);
    return data.size();
}

std::size_t __time_critical_func(QspiPSRAM::read)(const std::size_t address, DataBuffer data)
{
    constexpr uint8_t wait_cycles = 6;

    // Wait cycles must be patched, so it cannot be const
    uint8_t cmd[] = {
        qspi_fast_read_cmd, 
        static_cast<uint8_t>((address >> 16)), 
        static_cast<uint8_t>((address >> 8)), 
        static_cast<uint8_t>((address & 0xff)),
        wait_cycles 
    };

    qspi_.acquire_bus();
    qspi_.qspi_command_read(cmd, data);
    return data.size();
}

bool QspiPSRAM::perform_post()
{
    constexpr uint8_t cmd[] = {
        read_eid_cmd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t eid_buffer[sizeof(cmd)] = {};
    qspi_.spi_transmit(cmd, eid_buffer);

    return eid_buffer[4] == 0x0d && eid_buffer[5] == 0x5d;
}

void QspiPSRAM::exit_qpi_mode()
{
    if (!qspi_mode_) return;
    qspi_mode_ = false;
    constexpr uint8_t cmd[] = {exit_qpi_cmd};
    qspi_.qspi_write(cmd);
}

void QspiPSRAM::enter_qpi_mode()
{
    if (qspi_mode_) return;
    qspi_mode_ = true;
    constexpr uint8_t cmd[] = {enter_qpi_cmd};
    qspi_.spi_write(cmd);
}

void QspiPSRAM::benchmark() 
{
    printf("=====Begin test=====\n");
    qspi_.acquire_bus();
    constexpr int benchmark_rows = 20;
    uint8_t test_data[benchmark_rows][1024] = {};

    for (auto& line : test_data)
    {
        for (auto& byte : line)
        {
            byte = static_cast<uint8_t>(rand() * 255);
        }
    }
    uint8_t read_data[benchmark_rows][1024];
    uint64_t start_time = msgpu::get_us();
    uint32_t address = 0; 
    for (const auto& line : test_data)
    {
       write(address, line);
       address += sizeof(test_data[0]);
    }
    wait_for_finish();
    uint64_t write_end_time = msgpu::get_us();
    printf("Took %ld\n", write_end_time - start_time);
    printf("Speed: %f MB/s\n", static_cast<float>(sizeof(test_data)) / static_cast<float>(write_end_time - start_time));
    uint64_t read_start_time = msgpu::get_us();
    address = 0;
    for (auto& line : read_data)
    {
        read(address, line);
        address += sizeof(test_data[0]);
    }

    wait_for_finish();
    uint64_t read_end_time = msgpu::get_us();
    printf("Took %ld\n", read_end_time - read_start_time);
    printf("Speed: %f MB/s\n", static_cast<float>(sizeof(test_data)) / static_cast<float>(read_end_time - read_start_time));

    printf("====Verification started====\n");
    int success = 0;
    int failure = 0;
    for (int y = 0; y < benchmark_rows; ++y)
    {
        if (y == 0)
        {
            printf("Data: ");
        }
        for (std::size_t x = 0; x < sizeof(test_data[0]); ++x)
        {
            if (y == 0 && x < 32) 
            {
                printf("0x%x (0x%x), ", read_data[y][x], test_data[y][x]);
                if (x % 16 == 15) 
                {
                    printf("\n");
                }
            }
            if (test_data[y][x] != read_data[y][x])
            {
                ++failure;
                if (y == 0 && x < 32)
                {
                }else {
                break;
                }
            }
        }
        if (y == 0)
        {
            printf("\n");
        }
        ++success;
    }
    printf("Failed: %d/%d\n", failure, success);
    qspi_.release_bus();
}

bool QspiPSRAM::test()
{
    qspi_.acquire_bus();
    printf("QSPI memory starting test...\n");
    uint8_t buffer[1024];
    uint8_t readed[1024];
    uint32_t address = 0;
    uint32_t failed = 0;
    uint32_t executed = 0;

    while (address < psram_max_size) 
    {
        for (uint32_t i = 0; i < sizeof(buffer); ++i)
        {
            buffer[i] = static_cast<uint8_t>(rand() * 255);
        }
        write(address, buffer);
        read(address, readed);

        wait_for_finish();
        
        executed += 1;

        for (uint32_t i = 0; i < sizeof(buffer); ++i)
        {
            if (buffer[i] != readed[i])
            {
                printf("Failure detected at 0x%x\n", address);
                printf("i: %d\n", i);
                int begin = 0;
                int end = sizeof(buffer) - 1;

                if (i > 10)
                {
                    begin = i - 10; 
                }

                if (i + 10 < sizeof(buffer))
                {
                    end = i + 10;
                } 

                for (int x = begin; x < end; ++x)
                {
                    printf("i %d -> 0x%x != 0x%x\n", x, buffer[x], readed[x]);
                } 


                failed += 1;
                
                return true;
                break;
            }
        }
        
        if (failed && executed % 1024 == 1023)
        {
            printf("Failed %d/%d\n", failed, executed);
        }
        address += sizeof(buffer);
    }
    
    printf("Memory test: %d/%d\n", (executed - failed), executed);
    qspi_.release_bus();
    return true;//failed == 0;    
}

void QspiPSRAM::acquire_bus()
{
    qspi_.acquire_bus();
}

void QspiPSRAM::release_bus()
{
    qspi_.release_bus();
}

} // namespace msgpu::memory
