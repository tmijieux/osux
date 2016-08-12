#!/usr/bin/env python3

import gi, signal
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

def on_click(button):
    #Toggle visibility
    if popover.get_visible():
        popover.hide()
    else:
        popover.show_all()

if __name__ == "__main__":
    #Creating the window
    window = Gtk.Window(title="Hello Popover")
    window.connect("destroy", lambda w: Gtk.main_quit())
    
    #Creating and placing a button.
    box = Gtk.Box(spacing = 60)
    button = Gtk.Button("Toggle popover")
    button.connect("clicked", on_click)
    box.add(button)
    window.add(box)
    
    #Creating a popover
    popover = Gtk.Popover.new(button)
    popover.set_size_request(50,100)
    
    box = Gtk.Box(spacing = 10, orientation="vertical")
    search = Gtk.SearchEntry()
    box.add(search)
    label = Gtk.Label("Hi!")
    box.add(label)
    popover.add(box)
    window.set_size_request(400,200)
    window.show_all()

    signal.signal(signal.SIGINT, signal.SIG_DFL)
    Gtk.main()
