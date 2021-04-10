#!/bin/python3 

import argparse 
import tkinter as tk 
from tkinter import ttk 
import serial 

parser = argparse.ArgumentParser(description="Script to testing msgpu with GUI")

parser.add_argument("--serial", dest="serial", action="store", 
        help="Serial port device", required=True)

args, rest = parser.parse_known_args() 

ser = serial.Serial(args.serial, 115200)

window = tk.Tk()

def donothing():
    pass

tab_control = ttk.Notebook(window)
tab1 = ttk.Frame(tab_control)
tab2 = ttk.Frame(tab_control)
tab3 = ttk.Frame(tab_control)

tab_control.add(tab1, text="text mode")
tab_control.add(tab2, text="graphic mode")
tab_control.add(tab3, text="settings")

tab_control.pack(expand=1, fill="both")

text_box = tk.Text(tab1) 

def send_text(event):
    text = text_box.get("1.0", tk.END)
    ser.write("write\n".encode())
    ser.write(text.replace("\n", "\r").encode()) 
    ser.write([27, 27])
    text_box.delete("1.0", tk.END)

def clear_screen(event):
    ser.write("clear\n".encode())

text_box.pack()

frame = tk.Frame(tab1, relief=tk.RAISED, borderwidth=1)
frame.pack(fill=tk.BOTH, expand=True)

send_button = tk.Button(tab1, text="Send", width=25, height=5)
send_button.pack(side=tk.RIGHT, padx=5, pady=5)

send_button.bind("<Button-1>", send_text)

clear_button = tk.Button(tab1, text="Clear screen", width = 25, height = 5)
clear_button.pack(side=tk.RIGHT)
clear_button.bind("<Button-1>", clear_screen)

print ("Hello")
window.mainloop()
