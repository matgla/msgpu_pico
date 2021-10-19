from array import array
import time
import os
from typing import Set

from unittest import TestCase

from tests.TestFramework import SUT

from messages.allocate_program import AllocateProgramRequest, AllocateProgramType
from messages.program_write import ProgramWrite
from messages.begin_program_write import BeginProgramWrite
from messages.attach_shader import AttachShader
from messages.use_program import UseProgram
from messages.bind import BindObject, BindObjectType

from messages.generate_names import GenerateNamesRequest, ObjectType
from messages.buffer_target_type import BufferTargetType
from messages.write_buffer_data import PrepareForData, WriteBufferData, BufferDataUsage
from messages.draw_arrays import DrawArrayMode, DrawArrays
from messages.swap_buffer import SwapBuffer
from messages.set_vertex_attrib import SetVertexAttrib

import config_tests


class NamedParametersShouldBePassedToShader(TestCase):
    def setUp(self) -> None:
        self.sut = SUT(config_tests.path_to_msgpu)
        pass

    def tearDown(self) -> None:
        self.sut.close()
        pass

    def write_triangle(self):
        msg = GenerateNamesRequest()
        msg.type = ObjectType.values["VertexArray"]
        msg.elements = 1
        self.sut.gpu_io().write(msg)
        vao = self.sut.gpu_io().read()
        vao = vao.data[0]

        msg.type = ObjectType.values["Buffer"]
        self.sut.gpu_io().write(msg)
        vbo = self.sut.gpu_io().read()
        vbo = vbo.data[0]

        bind_msg = BindObject()
        bind_msg.type = BindObjectType.values["BindBuffer"]
        bind_msg.target = BufferTargetType.values["ArrayBuffer"]
        bind_msg.object_id = vbo
        self.sut.gpu_io().write(bind_msg)

        verticies = array(
            'f', [0.0, 0.5, 0.0, 0.5, -0.5, 0.0, -0.5, -0.5, 0.0])

        encoded_verticies = verticies.tobytes()
        self.write_buffer(0, encoded_verticies)

        return vao

    def write_buffer(self, buffer_id, data):
        msg = PrepareForData()
        msg.named = False
        msg.target = BufferTargetType.values["ArrayBuffer"]
        msg.usage = BufferDataUsage.values["StaticDraw"]
        msg.size = len(data)
        self.sut.gpu_io().write(msg)

        index = 0
        part = 0
        size = len(data)
        chunk_max_size = len(WriteBufferData().data)
        while index < size:
            chunk_size = size - index
            if chunk_size < chunk_max_size:
                chunk_size = size - index
            else:
                chunk_size = chunk_max_size

            chunk = data[index:index+chunk_size]
            index += chunk_size
            msg = WriteBufferData()
            msg.size = chunk_size
            msg.part = part
            part += 1
            msg.data = chunk
            self.sut.gpu_io().write(msg)

    def write_program(self, program_id, path):
        msg = BeginProgramWrite()
        msg.program_id = program_id
        msg.size = os.path.getsize(path)
        self.sut.gpu_io().write(msg)
        chunk_size = len(ProgramWrite().data)
        with open(path, "rb") as file:

            while True:
                chunk = file.read(chunk_size)
                if not chunk:
                    break
                msg = ProgramWrite()
                msg.size = len(chunk)
                msg.data = chunk
                msg.part = 0
                self.sut.gpu_io().write(msg)

    def write_shader(self, shader_path, program_id, type):
        req = AllocateProgramRequest()
        req.program_type = AllocateProgramType.values[type]
        self.sut.gpu_io().write(req)
        shader_allocation_response = self.sut.gpu_io().read()
        self.write_program(shader_allocation_response.program_id, shader_path)

        msg = AttachShader()
        msg.program_id = program_id
        msg.shader_id = shader_allocation_response.program_id
        self.sut.gpu_io().write(msg)

    def write_pixel_shader(self, shader_path, program_id):
        self.write_shader(shader_path, program_id, "AllocateFragmentShader")

    def write_vertex_shader(self, shader_path, program_id):
        self.write_shader(shader_path, program_id, "AllocateVertexShader")

    def set_attribute(self, ):
        req = SetVertexAttrib()
        req.index = 0
        req.size = 3
        req.normalized = False
        req.type = 1
        req.stride = 3 * 4
        req.pointer = 0
        self.sut.gpu_io().write(req)

    def test(self):
        req = AllocateProgramRequest()
        req.program_type = AllocateProgramType.values["AllocateProgram"]
        self.sut.gpu_io().write(req)
        program_allocation_response = self.sut.gpu_io().read()
        pid = program_allocation_response.program_id

        self.write_vertex_shader(config_tests.path_to_vertex_shader, pid)
        self.write_pixel_shader(config_tests.path_to_pixel_shader, pid)

        msg = UseProgram()
        msg.program_id = pid
        self.sut.gpu_io().write(msg)

        vao = self.write_triangle()
        self.set_attribute()
        msg = DrawArrays()
        msg.mode = DrawArrayMode.values["Triangles"]
        msg.count = 3
        msg.first = vao
        self.sut.gpu_io().write(msg)

        msg = SwapBuffer()
        self.sut.gpu_io().write(msg)

        swap = self.sut.i2c_io().read(2)
        print(swap)

        ack = bytearray([0xac, 0x88])
        self.sut.i2c_io().write(ack)

        ack = self.sut.gpu_io().read()
        print(ack)

        self.sut.gpu_io().dump_frame()
