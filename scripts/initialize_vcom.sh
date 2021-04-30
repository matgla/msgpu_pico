#!/bin/bash 

socat -d -d pty,raw,echo=0,link=/tmp/msgpu_virtual_serial_0 pty,raw,echo=0,link=/tmp/msgpu_virtual_serial_1 &
