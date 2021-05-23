#!/bin/python3 

import serial 

from pycparser import c_parser, c_ast, parse_file

import argparse 
import pathlib

argparser = argparse.ArgumentParser(description="Script to read msgpu info")

argparser.add_argument("--bytes", help="Path to directory with interface")
argparser.add_argument("--start", help="Path to directory with interface")
argparser.add_argument("--data")

args = argparser.parse_args()

ser = serial.Serial("/dev/ttyUSB0", 115200)

if args.start:
    data = list(range(int(args.start), int(args.start) + int(args.bytes)))
else: 
    data = eval(args.data)

print(data)
ser.write(data)
# ser.write([1])

# print("Getting info")
# print(ser.read(5))
