import st_config
import time

from tests.text_test_base import TextTestBase

from i2c_messages.i2c_messages import i2c_ack, i2c_swap_id


class MsgpuShouldAllowUseTextMode(TextTestBase):
    def test_simple_text(self):
        self.change_mode(1)
        self.write_text("Hello World!")

        time.sleep(0.2)
