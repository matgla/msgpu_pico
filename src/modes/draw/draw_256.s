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

.global draw_256
draw_256:
    // prepare
    push {r2}

    .rept 260 // align to center of screen
    nop
    .endr
    .rept 256
        ldrbt r2, [r0]  // load first pixel // 2C
        strb r2, [r1]   // store to odr // 2C
        add r0, #1
    .endr

    mov r2, #0x00 // 1C
    nop
    nop
    strb r2, [r1] // 2C

    pop {r2}
    bx lr

