"""
Run gdb with:

gdb -ex 'source factory-test.py' -ex 'target remote localhost:3333' -ex 'break orchardShellRestart' -ex 'continue' build/orchard.elf
"""
import gdb
import sys
import gtk

def run_test(test_name, test_type):
    tests = list_tests()
    test_index = tests.index(test_name)
    testidx = "first_test[" + str(test_index) + "]"
    cmd = "call "+testidx+".test_function("+testidx+", " + str(test_type) + ")"

    result = gdb.execute(cmd, False, True)
    for line in result.split("\n"):
        if line.startswith("$"):
#            print("Evaluating line: " + line)
#            return gdb.parse_and_eval(line.split(' ')[0])
            return line.split(' ')[2].replace("orchardResult", "")
    return None

def list_tests():
    tests = []
    i = 0
    while True:
        val = gdb.parse_and_eval("first_test[" + str(i) + "].test_name")
        if int(val.referenced_value()) == 0:
            break
        else:
            tests.append(val.string())
            i = i + 1
    return tests

class OrchardCall(gdb.Command):
    def __init__(self):
        super(OrchardCall, self).__init__(
                name = "orchardcall",
                command_class = gdb.COMMAND_DATA,
                completer_class = gdb.COMPLETE_SYMBOL
            )
    def invoke(self, argument, from_tty):
        print("Greetings again from Python")
        list_tests()

OrchardCall()

from threading import Thread
import gtk

class TestGtkThread(Thread):
    def destroy(self, *args):
        self.window.hide()

    def runtest(self, button, arg):
        testname = button.get_label()

        self.result_label.set_label("Testing " + testname + "...")

        # Force a redraw
        gtk.main_iteration()

        result = run_test(testname, arg)
        self.result_label.set_label("Testing " + testname + ": " + result)

    def run(self):
        gtk.gdk.threads_init()

        tests = list_tests()

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("destroy", self.destroy)
        self.window.set_border_width(10)
        self.window.set_title("Orchard Tests")

        table = gtk.Table(4, len(tests) + 2, False)
        row = 0
        self.window.add(table)

        table.attach(gtk.Label("Poweron"),       0, 1, row, row + 1)
        table.attach(gtk.Label("Trivial"),       1, 2, row, row + 1)
        table.attach(gtk.Label("Comprehensive"), 2, 3, row, row + 1)
        table.attach(gtk.Label("Interactive"),   3, 4, row, row + 1)
        row = row + 1

        for test in tests:
            button = gtk.Button(test)
            button.connect("clicked", self.runtest, 0)
            table.attach(button, 0, 1, row, row + 1)

            button = gtk.Button(test)
            button.connect("clicked", self.runtest, 1)
            table.attach(button, 1, 2, row, row + 1)

            button = gtk.Button(test)
            button.connect("clicked", self.runtest, 2)
            table.attach(button, 2, 3, row, row + 1)

            button = gtk.Button(test)
            button.connect("clicked", self.runtest, 3)
            table.attach(button, 3, 4, row, row + 1)
            row = row + 1

        self.result_label = gtk.Label("")
        table.attach(self.result_label, 0, 4, row, row + 1)
        row = row + 1

        self.window.show_all()
        gtk.main()

class TestPane(gdb.Command):
    def __init__ (self):
        super(TestPane, self).__init__ ("testpane",
                                        gdb.COMMAND_NONE,
                                        gdb.COMPLETE_NONE)
        self.init = False

    def invoke(self, arg, from_tty):
        self.dont_repeat()
        gdb.execute("set pagination off")
#        gdb.execute("mon reset halt")
#        gdb.execute("continue")
        if not self.init:
            self.init = True
            v = TestGtkThread()
            v.setDaemon(True)
            v.start()

TestPane()
