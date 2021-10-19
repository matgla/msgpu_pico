import multiprocessing
import os
import time
import binascii
import struct
import select
import numpy as np
import png

from messages.header import Header
from messages.messages import Messages
from messages import *
from tests.log import Logger

from multiprocessing import shared_memory

import sys


class GpuInterface:
    start_token = struct.pack("B", 0x7e)

    def __init__(self, gpu_in_file, gpu_out_file):
        counter = 0
        self._gpuin = None
        self._gpuout = None
        self._logger = Logger("GpuIO")
        while counter < 5:
            if self._gpuout == None and os.path.exists(gpu_out_file):
                self._gpuout_fd = os.open(gpu_out_file, os.O_WRONLY)
                self._gpuout = os.fdopen(self._gpuout_fd, "wb")
                self._gpuout.flush()
                self._logger.log("Gpu out initialized from:", gpu_out_file)
            if self._gpuin == None and os.path.exists(gpu_in_file) and self._gpuout != None:
                self._gpuin = open(gpu_in_file, "rb")
                self._gpuin.flush()
                self._logger.log("Gpu input initialized from:", gpu_in_file)
            if self._gpuin != None and self._gpuout != None:
                break

            time.sleep(0.01)
            counter = counter + 1

        self.initialize_gpu_shared_memory()

    def initialize_gpu_shared_memory(self):
        self._shm = shared_memory.SharedMemory(
            name="qspi_framebuffer_out")

        self._memory = self._shm.buf

    def from_rgb332(self, value):
        r = (value >> 5) & 0x07
        g = (value >> 2) & 0x07
        b = (value & 0x03)

        r = r * (256 / 8)
        g = g * (256 / 8)
        b = b * (256 / 4)

        return (int(r), int(g), int(b))

    def dump_frame(self):
        height = 240
        width = 320
        img = []
        for y in range(height):
            row = ()
            index = 1024*y
            for x in range(width):
                row = row + self.from_rgb332(self._memory[index])
                index += 2
            img.append(row)
        with open("dump.png", "wb") as f:
            w = png.Writer(width, height, greyscale=False)
            w.write(f, img)

    def _calculate_crc(self, data):
        return binascii.crc_hqx(data, 0x0000)

    def close(self):
        self._gpuin.close()
        self._gpuout.close()
        del self._memory
        self._shm.close()
        self._shm.unlink()

    def write(self, msg):
        h = Header()
        h.id = getattr(Messages, msg._type.name)
        payload = msg.dumps()
        h.size = len(payload)

        header_crc = self._calculate_crc(h.dumps())
        self._gpuout.write(GpuInterface.start_token)
        self._gpuout.flush()
        self._gpuout.write(h.dumps())
        self._gpuout.write(struct.pack("H", header_crc))
        self._gpuout.flush()
        self._gpuout.write(payload)
        payload_crc = self._calculate_crc(payload)
        self._gpuout.write(struct.pack("H", payload_crc))
        self._gpuout.flush()

    def _get_message_name(self, id):
        for key, value in vars(Messages).items():
            if type(value) == int:
                if id == value:
                    return key
        return None

    def read(self):
        b = 0
        counter = 0
        while b != GpuInterface.start_token:
            b = self._gpuin.read(1)

        print("GOT token")
        payload = self._gpuin.read(len(Header))
        h = Header(payload)
        print(h)

        h_crc = struct.unpack("H", self._gpuin.read(2))[0]
        calculated_crc = self._calculate_crc(payload)
        if h_crc != calculated_crc:
            print("CRC verification failed, got: ", hex(
                h_crc), ", calculated: ", hex(calculated_crc))
            assert(False)

        data_payload = self._gpuin.read(h.size)

        data_crc = struct.unpack("H", self._gpuin.read(2))[0]
        calculated_crc = self._calculate_crc(data_payload)
        if data_crc != calculated_crc:
            print("CRC verification failed, got: ", hex(
                data_crc), ", calculated: ", hex(calculated_crc))
            assert(False)

        msg_type = self._get_message_name(h.id)
        message_class = globals()[msg_type]
        msg = message_class(data_payload)
        return msg
