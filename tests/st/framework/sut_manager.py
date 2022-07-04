import subprocess


def load_binary(path):
    p = subprocess.Popen(path, shell=False)  # , stdout=subprocess.PIPE)
    return p
