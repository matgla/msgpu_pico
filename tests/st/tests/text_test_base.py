from tests.test_base import TestBase

from messages.write_text import WriteText


class TextTestBase(TestBase):
    def write_text(self, msg):
        req = WriteText()
        sent_size = 0
        to_sent_size = len(msg)
        data_size = len(req.data)
        while sent_size < to_sent_size:
            part_size = min(to_sent_size - sent_size, data_size)
            msg_part = msg[sent_size:sent_size + part_size]
            req.data = bytes(msg_part, encoding='utf-8')
            sent_size += part_size
            self.sut.gpu_io().write(req)
