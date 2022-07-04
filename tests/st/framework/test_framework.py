import os

from framework.sut_manager import load_binary
from framework.gpu_interface import GpuInterface
from framework.i2c_interface import I2CInterface

import st_config

from tests.log import Logger, log


class SUT:
    def __init__(self, path):
        self._logger = Logger("SUT")
        self._logger.log("Open application:", path)

        self._clear_io()

        self._binary = load_binary(path)

        # order is important, GPUIO waits for streams created by SUT
        self._logger.log("Initialize GPU/IO")
        self._gpuio = GpuInterface(
            st_config.gpu_io_in_path, st_config.gpu_io_out_path)

        self._logger.log("Initialize I2C")
        self._i2c = I2CInterface(
            st_config.i2c_r_path, st_config.i2c_w_path
        )

    def _clear_io(self):
        for file in [st_config.gpu_io_in_path, st_config.gpu_io_out_path, st_config.i2c_r_path, st_config.i2c_w_path]:
            if os.path.exists(file):
                os.remove(file)

    def close(self):
        self._logger.log("Close application")
        self._binary.terminate()
        if self._binary.stdout:
            self._binary.stdout.close()
        self._gpuio.close()
        self._i2c.close()
        self._binary.wait()

    def gpu_io(self):
        return self._gpuio

    def i2c_io(self):
        return self._i2c
