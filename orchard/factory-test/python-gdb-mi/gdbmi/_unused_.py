class MiFrame(object):
    def __init__(self):
        self.level = None # level  from 0
        self.func = None # function corresponding to the frame.
        self.addr = None # code address
        self.file = None # name of the source file
        self.line = None # source line
        self.binary = None # name of the binary file
 
class MiCommand(object):
    def __init__(self, op, opts, params = None):
        self.op = op
        self.opts = opts
        self.params = params

    def encode(self):
        """
        >>> MiCommand("test", []).encode()
        '-test\\n'
        """
        if self.params is None:
            param_str = ""
        else:
            param_str = " -- " + " ".join(self.params)
        return ("-" +
                " ".join([self.op] + self.opts) +
                param_str + "\n")
