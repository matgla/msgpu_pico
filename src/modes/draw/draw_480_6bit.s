// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
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


.syntax unified
.arch armv7-m
.thumb

// Draw optimized version of 400 pixel width
.global draw_480_6bit
draw_480_6bit:
    // prepare
    push {r2}

    .rept 60
        nop
    .endr

    .rept 96
        ldr r2, [r0] // load first 32 pixels 2C
        nop
        strb r2, [r1]  // store to odr // 2C

        .rept 3 // 3 next pixels in uint32_t
            ror r2, r2, #0x6 // 1C
            nop
            strb r2, [r1]    // 2C
        .endr

        // last pixel differs since data pointer must be incremented
        ror r2, r2, #0x6 // 1C
        add r0, #4       // 1C
        strb r2, [r1]    // 2C
    .endr

    mov r2, #0x00 // 1C
    nop
    nop
    strb r2, [r1] // 2C

    pop {r2}
    bx lr

