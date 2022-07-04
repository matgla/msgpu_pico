import sys
import pathlib
import os

path_to_config = os.environ.get("st_config_path")
print(path_to_config)
sys.path.append(path_to_config)

from st_config import path_to_messages_interface

path = os.path.join(str(pathlib.Path(
    __file__).parent.resolve()) + "/framework")
sys.path.append(path)
sys.path.append(path_to_messages_interface)


path = os.path.join(str(pathlib.Path(
    __file__).parent.resolve()) + "/framework")
sys.path.append(path)
sys.path.append(path_to_messages_interface)
