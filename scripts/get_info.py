#!/bin/python3 

import serial 

import argparse 
import pathlib
import struct 
import ctypes
import construct

from dissect import cstruct

argparser = argparse.ArgumentParser(description="Script to read msgpu info")

argparser.add_argument("--interface", help="Path to directory with interface")
argparser.add_argument("--serial", help="MSGPU serial port", required=True)
args = argparser.parse_args()

ser = serial.Serial(args.serial, 115200)
ser.write([1])

print("Getting info")
id = ser.read(1)
print("Message id: ", id)

cparser = cstruct.cstruct()
cparser.load(""" 
    #define MAX_MODES 16 
    
    typedef struct {
        uint8 uses_color_palette : 1;
        uint8 mode : 1;
        uint8 id : 6;
        uint16 resolution_width;
        uint16 resolution_height;
        uint16 color_depth;
    } mode_info;

    typedef struct {
        uint8 version_major;
        uint8 version_minor;
        mode_info modes[MAX_MODES];
    } info_resp;
""")

message_size = len(cparser.info_resp)
print("message size: ", message_size)
payload = ser.read(message_size)
message = cparser.info_resp(payload)
print (message)
