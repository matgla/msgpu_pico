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

#include "disk/disk.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <hardware/flash.h>
#include <hardware/sync.h>

extern "C" 
{

extern uint8_t __disk;

}

namespace 
{

int flash_read(const lfs_config* c, lfs_block_t block, 
    lfs_off_t off, void* buffer, lfs_size_t size)
{
    const uint8_t* disk = &__disk + block * FLASH_SECTOR_SIZE + off;
    std::memcpy(buffer, disk, size);

    return 0;
}

static const std::size_t flash_drive_offset = reinterpret_cast<std::size_t>(&__disk) - XIP_BASE;

int flash_program(const lfs_config* c, lfs_block_t block, 
    lfs_off_t off, const void* buffer, lfs_size_t size)
{
    const uint32_t page_address = flash_drive_offset + block * FLASH_SECTOR_SIZE + off;
    printf("Program at: 0x%x\n", page_address);
    const uint32_t mask = save_and_disable_interrupts();
    flash_range_program(page_address, static_cast<const uint8_t*>(buffer), size);
    restore_interrupts(mask);
    return 0;
}

int flash_erase(const lfs_config* c, lfs_block_t block)
{
    const uint32_t block_address = flash_drive_offset + block * FLASH_SECTOR_SIZE; 
    printf("Erase at: 0x%x\n", block_address);
    const uint32_t mask = save_and_disable_interrupts();
    flash_range_erase(block_address, FLASH_SECTOR_SIZE);
    restore_interrupts(mask);

    return 0;
}

int flash_sync(const lfs_config* c)
{
    return 0;
}

} // namespace

namespace disk 
{

void Disk::load() 
{
    printf("Loading disk at: 0x%x\n", &__disk);
    initialize_drive();
}

void Disk::initialize_drive()
{
    const lfs_config config = {
        .read = &flash_read, 
        .prog = &flash_program, 
        .erase = &flash_erase,
        .sync = &flash_sync, 
        .read_size = sizeof(uint8_t),
        .prog_size = FLASH_PAGE_SIZE,
        .block_size = FLASH_SECTOR_SIZE,
        .block_count = 256, 
        .block_cycles = -1, 
        .cache_size = FLASH_PAGE_SIZE,
        .lookahead_size = FLASH_PAGE_SIZE
    };

    int err = lfs_mount(&lfs_, &config);
    if (err) 
    {
        printf("Mouting failed, formating...\n");
        lfs_format(&lfs_, &config);
        lfs_mount(&lfs_, &config);
    }

    uint32_t boot_count = 0;
    
    lfs_file_t file;
    lfs_file_open(&lfs_, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs_, &file, &boot_count, sizeof(boot_count));
    
    printf("Boot counter: %d\n", boot_count);
    ++boot_count; 

    printf("Rewind\n");
    lfs_file_rewind(&lfs_, &file);
    printf("Write\n");

    printf("Work done unmounting...\n");
    lfs_unmount(&lfs_);
}

}
