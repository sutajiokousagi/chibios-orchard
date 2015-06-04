"""
Run gdb with:

gdb -ex 'source test.py' -ex 'target remote localhost:3333' build/orchard.elf
"""
import gdb
import sys
import gtk

class OrchardCall(gdb.Command):
    def __init__(self):
        super(OrchardCall, self).__init__(
                name = "orchardcall",
                command_class = gdb.COMMAND_DATA,
                completer_class = gdb.COMPLETE_SYMBOL
            )
    def invoke(self, argument, from_tty):
        print("Greetings again from Python")
        result = gdb.execute("call captouchRead()", False, True)
        print(">> " + result + " <<")
        convenience_arg = result.split(' ')[0]
        print("Convenience arg is: " + convenience_arg)

        val = gdb.parse_and_eval(convenience_arg)
        print("Val type code is: " + val.type.name)

OrchardCall()

from threading import Thread
import gtk

def printit ():
    print "Hello hacker"
    print(">>" + gdb.execute("call captouchRead()", False, True) + "<<")

class TestGtkThread (Thread):
    def destroy (self, *args):
        self.window.hide()

    def hello (self, *args):
        gdb.post_event(printit)

    def run (self):
        gtk.gdk.threads_init()

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("destroy", self.destroy)
        self.window.set_border_width(10)

        button = gtk.Button("Hello World")
        # connects the 'hello' function to the clicked signal from the button
        button.connect("clicked", self.hello)
        self.window.add(button)
        button.show()

        self.window.show_all()
        gtk.main()

class TestGtk (gdb.Command):
    def __init__ (self):
        super (TestGtk, self).__init__ ("testgtk", gdb.COMMAND_NONE,
                                         gdb.COMPLETE_NONE)
        self.init = False

    def invoke (self, arg, from_tty):
        self.dont_repeat()
        if not self.init:
            self.init = True
            v = TestGtkThread()
            v.setDaemon (True)
            v.start ()

TestGtk()
