#!/bin/python3
import gi
import signal
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GdkPixbuf, Gdk

class Handler:
    def hello(self, button):
        print("Hello World!")

    def gtk_main_quit(self, button):
        Gtk.main_quit()
    
    def gl_fini(self, a, b):
        pass

    def gl_init(self, a, b):
        pass

    def gl_render(self, a, b):
        pass

    def gl_draw(self, a, b):
        pass

builder = Gtk.Builder()
builder.add_from_file("ui/interface.glade")
window = builder.get_object("window1")
window.connect("delete-event", Gtk.main_quit)
builder.connect_signals(Handler())
window.show_all()
signal.signal(signal.SIGINT, signal.SIG_DFL)
Gtk.main()
