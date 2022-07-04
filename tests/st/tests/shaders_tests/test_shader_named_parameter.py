import st_config

from tests.test_base import TestBase

from i2c_messages.i2c_messages import i2c_ack, i2c_swap_id


class NamedParametersShouldBePassedToShader(TestBase):
    def test(self):
        pid = self.allocate_program()
        self.add_vertex_shader(
            st_config.path_to_binaries + "/shaders_tests/libvertex_shader_stub.so", pid)
        self.add_pixel_shader(
            st_config.path_to_binaries + "/shaders_tests/libpixel_shader_stub.so", pid)

        self.use_program(pid)

        vao, vbo = self.write_object(
            [0.0, 0.5, 0.0, 0.5, -0.5, 0.0, -0.5, -0.5, 0.0])

        self.set_attribute(attribute_index=0, size=3,
                           normalized=False, type=1, stride=3*4, pointer=0)

        self.draw_arrays("Triangles", vao, 3)
        self.swap_buffer()

        self.sut.i2c_io().expect_msg(bytearray([i2c_swap_id, 0x01]))
        self.sut.i2c_io().write(i2c_ack)

        ack = self.sut.gpu_io().read()

        frame_to_dump = self.generate_frame_name(__file__, "test1")
        frame_to_verify = self.get_verification_frame_name(__file__, "test_1")

        self.sut.gpu_io().dump_frame(frame_to_dump, frame_to_verify)
