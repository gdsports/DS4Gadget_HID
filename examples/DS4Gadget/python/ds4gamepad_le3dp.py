#!/usr/bin/python3
"""
Read from Logitech Extreme 3D Pro and write to DS4Gadget.

LE3DP -> Raspberry Pi -> DS4Gadget -> PS4
"""
import os
from sys import exit
from struct import unpack
import array
from fcntl import ioctl
import serial
from ds4gpadserial import DS4GamepadSerial, DS4Button

ds4g = DS4GamepadSerial()
ds4g.begin(serial.Serial('/dev/ttyAMA0', 2000000, timeout=0))

# Open the Logitech Extreme 3D Pro
# joystick code based on https://gist.github.com/rdb/8864666
LE3DP = False
for fn in os.listdir('/dev/input'):
    if fn.startswith('js'):
        print('/dev/input/%s' % (fn))
        jsdev = open('/dev/input/' + fn, 'rb')
        buf = array.array('B', [0] * 64)
        ioctl(jsdev, 0x80006a13 + (0x10000 * len(buf)), buf) # JSIOCGNAME(len)
        js_name = buf.tobytes().rstrip(b'\x00').decode('utf-8').upper()
        print('Device name: %s' % js_name)
        if 'LOGITECH EXTREME 3D' in js_name:
            LE3DP = True
            break
        else:
            jsdev.close()

if not LE3DP:
    print('Logitech Extreme 3D Pro not found')
    exit(1)

print('Logitech Extreme 3D Pro found')

# Get number of axes and buttons
buf = array.array('B', [0])
ioctl(jsdev, 0x80016a11, buf) # JSIOCGAXES
num_axes = buf[0]
print('num_axes = %s' % num_axes)

buf = array.array('B', [0])
ioctl(jsdev, 0x80016a12, buf) # JSIOCGBUTTONS
num_buttons = buf[0]
print('num_buttons = %s' % num_buttons)

# Map LE3DP button numbers to DS4 gamepad buttons
# LE3DP buttons
# 0 = front trigger
# 1 = side thumb rest button
# 2 = top large left
# 3 = top large right
# 4 = top small left
# 5 = top small right
#
# Button array (2 rows, 3 columns) on base
#
# 7 9 11
# 6 8 10
#
BUTTON_MAP = array.array('B', [
    DS4Button.CIRCLE,
    DS4Button.CROSS,
    DS4Button.SQUARE,
    DS4Button.TRIANGLE,
    DS4Button.L1,
    DS4Button.R1,
    DS4Button.SHARE,
    DS4Button.OPTIONS,
    DS4Button.TPAD,
    DS4Button.LOGO,
    DS4Button.L2,
    DS4Button.R2])

left_stick_pressed = False
right_stick_pressed = False

while True:
    evbuf = jsdev.read(8)
    if evbuf:
        time, value, type, number = unpack('IhBB', evbuf)
        if type & 0x01: # button event
            button_out = BUTTON_MAP[number]
            if value:
                ds4g.press(button_out)
            else:
                ds4g.release(button_out)

        if type & 0x02: # axis event
            axis = ((value + 32767) >> 8)
            if axis == 127:
                axis = 128
            # Axes 0,1 left stick X,Y
            if number == 0:
                ds4g.leftXAxis(axis)
            elif number == 1:
                ds4g.leftYAxis(axis)
            # Axis 2 twist
            elif number == 2:
                if axis == 0:
                    ds4g.press(DS4Button.L3)
                    left_stick_pressed = True
                elif axis == 255:
                    ds4g.press(DS4Button.R3)
                    right_stick_pressed = True
                else:
                    if left_stick_pressed:
                        ds4g.release(DS4Button.L3)
                        left_stick_pressed = False
                    if right_stick_pressed:
                        ds4g.release(DS4Button.R3)
                        right_stick_pressed = False
            # Axis 3 throttle lever
#            elif number == 3:
#                Find a use for this!
            # Axes 4,5 directional pad X,Y
            elif number == 4:
                ds4g.rightXAxis(axis)
            elif number == 5:
                ds4g.rightYAxis(axis)
