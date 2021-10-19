#!/bin/python3

from messages.change_mode import ChangeMode
from messages.messages import Messages
from messages.info_req import InfoReq
from messages.mode import Mode
from messages.header import Header
from messages.write_text import WriteText
from messages.info_resp import InfoResp
import serial

import argparse
import pathlib
import sys

from dissect import cstruct

argparser = argparse.ArgumentParser(description="Script to read msgpu info")

argparser.add_argument("--interface", help="Path to directory with interface")
argparser.add_argument("--serial", help="MSGPU serial port", required=True)
args = argparser.parse_args()

sys.path.append(args.interface)


ser = serial.Serial(args.serial, 115200)


def send_message(message):
    header = Header()
    header.id = getattr(Messages, message._type.name)
    payload = message.dumps()
    header.size = len(payload)

    print(header.dumps())
    ser.write(header.dumps())
    if len(payload):
        ser.write(payload)


def get_type(id):
    for field in vars(Messages):
        if getattr(Messages, field) == id:
            return field


def get_message():
    payload = ser.read(len(Header))
    header = Header(payload)
    payload = ser.read(header.size)
    return globals()[get_type(header.id)](payload)


send_message(InfoReq())
print("Send info request")
info_message = get_message()
for mode in info_message.modes:
    if mode.used == 1:
        if mode.mode == Mode.Text:
            mode_text = "text"
        else:
            mode_text = "graphic"
        print(mode.id, ":", mode_text, str(mode.resolution_width) + "x" + str(mode.resolution_height) + ", color depth:",
              mode.color_depth, ", uses palette:", mode.uses_color_palette)

change_mode = ChangeMode()
change_mode.mode = 12
send_message(change_mode)
