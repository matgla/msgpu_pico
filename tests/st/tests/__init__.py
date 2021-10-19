import sys
import pathlib
import os

from config_tests import path_to_messages_interface

path = os.path.join(str(pathlib.Path(
    __file__).parent.parent.resolve()) + "/framework")
print("[TEST] Append path: ", path)
print("[TEST] Append path: ", path_to_messages_interface)
sys.path.append(path)
sys.path.append(path_to_messages_interface)
