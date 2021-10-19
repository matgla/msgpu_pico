import os

from SutManager import load_binary
from GpuInterface import GpuInterface
from I2CInterface import I2CInterface

import config_tests

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
            config_tests.gpu_io_in_path, config_tests.gpu_io_out_path)

        self._logger.log("Initialize I2C")
        self._i2c = I2CInterface(
            config_tests.i2c_r_path, config_tests.i2c_w_path
        )

    def _clear_io(self):
        for file in [config_tests.gpu_io_in_path, config_tests.gpu_io_out_path, config_tests.i2c_r_path, config_tests.i2c_w_path]:
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
