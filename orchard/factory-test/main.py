#!/usr/bin/env python3
"""
OpenOCD RPC example, covered by GNU GPLv3 or later
Copyright (C) 2014 Andreas Ortmann (ortmann@finf.uni-hannover.de)


Example output:
./ocd_rpc_example.py
echo says hi!

target state: halted
target halted due to debug-request, current mode: Thread
xPSR: 0x81000000 pc: 0x000009d6 msp: 0x10001f48

variable @ 0x00000000: 0x10001ff0

variable @ 0x10001ff0: 0xdeadc0de

memory (before): ['0xdeadc0de', '0x00000000', '0xaaaaaaaa', '0x00000023',
'0x00000042', '0x0000ffff']

memory (after): ['0x00000001', '0x00000000', '0xaaaaaaaa', '0x00000023',
'0x00000042', '0x0000ffff']

"""
import socket

import sys
sys.path.append("./python-gdb-mi") 
sys.path.append("./factory-test/python-gdb-mi") 
sys.path.append("./factory-test/python-gdb-mi") 
import gdbmi
import logging

def strToHex(data):
    return map(strToHex, data) if isinstance(data, list) else int(data, 16)

def hexify(data):
    return "<None>" if data is None else ("0x%08x" % data)

def compareData(a, b):
    num = 0
    for i, j in zip(a, b):
        if i != j:
            print("found difference (ix=%d): %d != %d" % (num, i, j))

        num += 1


class OpenOcd:
    TOKEN = '\x1a'
    def __init__(self, verbose=False):
        self.verbose = verbose
        self.tclRpcIp       = "127.0.0.1"
        self.tclRpcPort     = 6666
        self.bufferSize     = 4096
        self.COMMAND_TOKEN  = 0x1a

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def __enter__(self):
        self.sock.connect((self.tclRpcIp, self.tclRpcPort))
        return self

    def __exit__(self, type, value, traceback):
        try:
            self.send("exit")
        finally:
            self.sock.close()

    def send(self, cmd):
        """Send a command string to TCL RPC. Return the result that was read."""
        data = (cmd + "\x1a").encode("ascii")
        if self.verbose:
            print("<- ", data)

        self.sock.send(data)
        return self._recv()

    def _recv(self):
        """Read from the stream until the token (\x1a) was received."""
        data = bytes()
        while True:
            chunk = self.sock.recv(self.bufferSize)
            data += chunk
            if bytes(self.TOKEN, encoding="ascii") in chunk:
                break

        if self.verbose:
            print("-> ", data)

        data = data.decode("ascii").strip()
        data = data[:-1] # strip trailing \x1a

        return data

    def readVariable(self, address):
        raw = self.send("ocd_mdw 0x%x" % address).split(": ")
        return None if (len(raw) < 2) else strToHex(raw[1])

    def readMemory(self, wordLen, address, n):
        self.send("array unset output") # better to clear the array before
        self.send("mem2array output %d 0x%x %d" % (wordLen, address, n))

        output = self.send("ocd_echo $output").split(" ")

        return [int(output[2*i+1]) for i in range(len(output)//2)]

    def writeVariable(self, address, value):
        assert value is not None
        self.send("mww 0x%x 0x%x" % (address, value))

    def writeMemory(self, wordLen, address, n, data):
        array = " ".join(["%d 0x%x" % (a, b) for a, b in enumerate(data)])

        self.send("array unset myArray") # better to clear the array before
        self.send("array set myArray { %s }" % array)
        self.send("array2mem myArray 0x%x %s %d" % (wordLen, address, n))

if __name__ == "__main__":

    gdb = None
    should_exit = False
    run_function_results = {}

    def show(*args):
        print(*args, end="\n")

    def exit_on_connect_error(token, obj):
        logging.error(['Get result.  Token, obj', obj.what, token, obj])

    def print_status(token, obj):
        logging.error(['Get result.  Token, obj', obj.what, token, obj])

    def run_function_get_result(token, obj):
        logging.error(['Get result.  Token, obj', obj.what, token, obj])
        if obj.what != '"\\n"':
            res = obj.what
            # Remove the "$1 = " at the front
            res = '='.join(res.split('=')[1:]).lstrip()
            run_function_results[token] = res

    def run_function(descr):
        token = gdb.send("call " + descr, None, run_function_get_result)
        gdb.wait_for(token)
        gdb.wait_for(token)
        return run_function_results[token]

    def do_test(bpnum, obj):
        prot = "gpiox_get(1)"
        logging.error(['Result of calling ', prot, run_function(prot)])
        should_exit = True

    with OpenOcd() as ocd:
        ocd.send("klx.cpu curstate")

        print("Doing mass erase...")
        ocd.send("kinetis mdm mass_erase")

        print ("Trying to halt SOC")
        while True:
            ocd.send("sleep 1000")
            state = ocd.send("klx.cpu curstate")
            if state == "halted":
                print("State is halted: " + state)
                break
            print("Trying to halt again")
            ocd.send("reset halt")

        # Print out SDID
        show("SDID: %s" % (hexify(ocd.readVariable(0x40048024))))

        # Print out Flash FCFG1 and FCFG2
        show("FCFG1: %s" % (hexify(ocd.readVariable(0x4004804C))))
        show("FCFG2: %s" % (hexify(ocd.readVariable(0x40048050))))

        # Print out unique IDs
        show("UIDMH: %s" % (hexify(ocd.readVariable(0x40048058))))
        show("UIDML: %s" % (hexify(ocd.readVariable(0x4004805C))))
        show("UIDL: %s" % (hexify(ocd.readVariable(0x40048060))))

        print("Rewriting flash...")
        ocd.send("flash write_image build/orchard.elf")

        print("Starting GDB")

        logging.basicConfig(
            #level=logging.INFO,
            level=logging.DEBUG,
            #level=logging.ERROR,
            format='%(asctime)s '\
            '%(levelname)s '\
            '%(pathname)s:%(lineno)s '\
            '%(message)s')

        gdb = gdbmi.Session("build/orchard.elf")
        token = gdb.send("set unwindonsignal on")
        logging.debug(["Set window", token])
        gdb.wait_for(token)

        token = gdb.send("target remote localhost:3333", print_status, exit_on_connect_error)
        logging.debug(["Connect to localhost", token])
        gdb.wait_for(token)

        token = gdb.send("monitor reset halt", print_status, exit_on_connect_error)
        logging.debug(["Reset board", token])
        gdb.wait_for(token)

        # When orchardShellRestart is to be called, run the testcase instead
        gdb.wait_for(gdb.add_breakpoint("orchardShellRestart", do_test))
        token = gdb.send("continue")

        while not should_exit:
            gdb.wait_for(None)
