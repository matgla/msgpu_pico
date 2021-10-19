import ctypes 
import threading 
from time import sleep  

try:
    shader_lib = ctypes.CDLL("/home/mateusz/repos/msgpu/build_st/tests/st/tests/shaders_tests/libshader_stub.so")
except Exception as e:
    print("Failed to load: ", e)

print(ctypes.c_char_p.in_dll(shader_lib, 'parameter_a'))

import subprocess 

print("Loading executable")

def run_msgpu():
    subprocess.run("/home/mateusz/repos/msgpu/build_st/src/gpu/msgpu")

t = threading.Thread(target=run_msgpu)
t.start()

print("Executed process which waiting for inputs")

gpuin = open("/tmp/gpu_com", "w")
gpuout = open("/tmp/gpu_com_2", "r")

sleep(1);

print ("Init I2C in")
i2cout = open("/home/mateusz/repos/msgpu/build_st/i2c_bus_w", "r")
print ("Init I2C out")
i2cin = open("/home/mateusz/repos/msgpu/build_st/i2c_bus_r", "w")

print("Wait for join threads")
t.join()