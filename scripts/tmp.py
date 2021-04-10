#!/bin/python3 

import serial 

from pycparser import c_parser, c_ast, parse_file

import argparse 
import pathlib

argparser = argparse.ArgumentParser(description="Script to read msgpu info")

argparser.add_argument("--interface", help="Path to directory with interface")

args = argparser.parse_args()

files = []
for path in pathlib.Path(args.interface).rglob("*.h"):
    files.append(path)

for header in files:
    ast = parse_file(header, use_cpp=False)
    ast.show()

# ser = serial.Serial("/dev/ttyACM1", 115200)
# ser.write([1])

# print("Getting info")
# print(ser.read(5))
