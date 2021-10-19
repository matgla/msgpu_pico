import os
import time

from tests.log import Logger


class I2CInterface:
    def __init__(self, i2c_in_file, i2c_out_file):
        counter = 0
        self._i2c_in = None
        self._i2c_out = None
        self._logger = Logger("I2C")
        while counter < 5:
            if self._i2c_in == None and os.path.exists(i2c_in_file):
                self._logger.log("I2C input initialized from:", i2c_in_file)
                self._i2c_in_fd = os.open(i2c_in_file, os.O_RDONLY)
                self._i2c_in = os.fdopen(self._i2c_in_fd, "rb")
                self._i2c_in.flush()
            if self._i2c_out == None and os.path.exists(i2c_out_file):
                self._i2c_out_fd = os.open(i2c_out_file, os.O_WRONLY)
                self._i2c_out = os.fdopen(self._i2c_out_fd, "wb")
                self._i2c_out.flush()
                self._logger.log("I2C out initialized from:", i2c_out_file)
            if self._i2c_in != None and self._i2c_out != None:
                break

            time.sleep(0.01)
            counter = counter + 1

    def close(self):
        self._i2c_in.close()
        self._i2c_out.close()

    def read(self, size):
        return self._i2c_in.read(size)

    def write(self, data):
        self._i2c_out.write(data)
        self._i2c_out.flush()
