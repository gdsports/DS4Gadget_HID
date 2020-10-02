#!/usr/bin/python3
"""
Exercise DS4GamepadSerial class.
"""
from time import sleep
import serial
from ds4gpadserial import DS4GamepadSerial

DS4G = DS4GamepadSerial()
# TODO port name from command line
DS4G.begin(serial.Serial('/dev/ttyAMA0', 2000000, timeout=0))

while True:
    # Press and hold every button 0..13
    for x in range(0, 14):
        DS4G.press(x)
        sleep(0.1)
    sleep(1)
    # Release all buttons
    DS4G.releaseAll()
    sleep(1)
    # Press all 14 buttons at the same time
    DS4G.buttons(0x3fff)
    sleep(1)
    # Release all buttons
    DS4G.releaseAll()
    sleep(1)
    # Move directional pad in all directions
    # 0 = North, 1 = North-East, 2 = East, etc.
    for x in range(0, 8):
        DS4G.dPad(x)
        sleep(0.5)
    # Move directional pad to center
    DS4G.dPad(15)
