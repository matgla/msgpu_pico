def log(*argv, tag=None):
    if tag:
        print("[TEST/" + tag + "]", *argv)
    else:
        print("[TEST]", *argv)


class Logger:
    def __init__(self, tag):
        self._tag = tag

    def log(self, *argv):
        log(*argv, tag=self._tag)
