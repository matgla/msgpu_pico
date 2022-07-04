from array import array
import os
import pathlib

from unittest import TestCase

import st_config

from framework.test_framework import SUT

from messages.allocate_program import AllocateProgramRequest, AllocateProgramType
from messages.attach_shader import AttachShader
from messages.use_program import UseProgram
from messages.program_write import ProgramWrite
from messages.begin_program_write import BeginProgramWrite
from messages.draw_arrays import DrawArrayMode, DrawArrays
from messages.bind import BindObject, BindObjectType
from messages.generate_names import GenerateNamesRequest, ObjectType
from messages.buffer_target_type import BufferTargetType
from messages.write_buffer_data import PrepareForData, WriteBufferData, BufferDataUsage
from messages.swap_buffer import SwapBuffer
from messages.set_vertex_attrib import SetVertexAttrib
from messages.get_named_parameter_id import GetNamedParameterIdReq, GetNamedParameterIdResp
from messages.change_mode import ChangeMode


class TestBase(TestCase):
    def setUp(self) -> None:
        self.sut = SUT(st_config.path_to_msgpu)

    def tearDown(self) -> None:
        self.sut.close()

    def allocate_program(self):
        req = AllocateProgramRequest()
        req.program_type = AllocateProgramType.values["AllocateProgram"]
        self.sut.gpu_io().write(req)
        program_allocation_response = self.sut.gpu_io().read()
        return program_allocation_response.program_id

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

    def use_program(self, pid):
        msg = UseProgram()
        msg.program_id = pid
        self.sut.gpu_io().write(msg)

    def draw_arrays(self, draw_type, first, count):
        msg = DrawArrays()
        msg.mode = DrawArrayMode.values[draw_type]
        msg.count = count
        msg.first = first
        self.sut.gpu_io().write(msg)

    def add_pixel_shader(self, shader_path, program_id):
        self.write_shader(shader_path, program_id, "AllocateFragmentShader")

    def add_vertex_shader(self, shader_path, program_id):
        self.write_shader(shader_path, program_id, "AllocateVertexShader")

    def write_object(self, vertexes):
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

        verticies = array('f', vertexes)

        encoded_verticies = verticies.tobytes()
        self.write_buffer(0, encoded_verticies)

        return (vao, vbo)

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

    def set_attribute(self, attribute_index, size, normalized, type, stride, pointer):
        req = SetVertexAttrib()
        req.index = attribute_index
        req.size = size
        req.normalized = normalized
        req.type = type
        req.stride = stride
        req.pointer = pointer
        self.sut.gpu_io().write(req)

    def swap_buffer(self):
        msg = SwapBuffer()
        self.sut.gpu_io().write(msg)

    def generate_frame_name(self, file, name):
        test_name = os.path.splitext(file)[0].split('/')[-1]
        return test_name + "_" + name + ".png"

    def get_verification_frame_name(self, file, name):
        test_name = os.path.splitext(file)[0].split('/')[-1]
        parent = str(pathlib.Path(file).parent)
        return parent + "/" + test_name + "_" + name + ".png"

    def change_mode(self, mode):
        req = ChangeMode()
        req.mode = mode
        self.sut.gpu_io().write(req)
