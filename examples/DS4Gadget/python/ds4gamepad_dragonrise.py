#!/usr/bin/python3
"""
Read from Dragon Rise Arcade Joystick and write to DS4Gadget.

DRAJ -> Raspberry Pi -> DS4Gadget -> PS4?
"""
import os
import time
from sys import exit
from struct import unpack
import threading
import array
from fcntl import ioctl
import serial
from ds4gpadserial import DS4GamepadSerial, DS4Button, DS4DPad

ds4g = DS4GamepadSerial()
ds4g.begin(serial.Serial('/dev/ttyAMA0', 2000000, timeout=0))

# Open the DRAJ
# joystick code based on https://gist.github.com/rdb/8864666
js_num = 0;
for fn in os.listdir('/dev/input'):
    if fn.startswith('js'):
        print('/dev/input/%s' % (fn))
        jsdev = open('/dev/input/' + fn, 'rb')
        buf = array.array('B', [0] * 64)
        ioctl(jsdev, 0x80006a13 + (0x10000 * len(buf)), buf) # JSIOCGNAME(len)
        js_name = buf.tobytes().rstrip(b'\x00').decode('utf-8').upper()
        print('Device name: %s' % js_name)
        if 'DRAGONRISE INC.   GENERIC   USB  JOYSTICK' in js_name:
            js_num += 1
            if js_num == 1:
                js_left = jsdev
            elif js_num == 2:
                js_right = jsdev
        else:
            jsdev.close()

if js_num < 2:
    print('DRAGONRISE joysticks not found')
    exit(1)

print('DRAGONRISE joysticks found')

# Get number of axes and buttons
buf = array.array('B', [0])
ioctl(js_left, 0x80016a11, buf) # JSIOCGAXES
num_axes = buf[0]

buf = array.array('B', [0])
ioctl(js_left, 0x80016a12, buf) # JSIOCGBUTTONS
num_buttons = buf[0]
print('left num_axes = %s num_buttons = %s' % (num_axes, num_buttons))

# Get number of axes and buttons
buf = array.array('B', [0])
ioctl(js_right, 0x80016a11, buf) # JSIOCGAXES
num_axes = buf[0]

buf = array.array('B', [0])
ioctl(js_right, 0x80016a12, buf) # JSIOCGBUTTONS
num_buttons = buf[0]
print('right num_axes = %s num_buttons = %s' % (num_axes, num_buttons))

# Map DRAJ button numbers to DS4 gamepad buttons
# DRAJ buttons
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
BUTTON_MAP_LEFT = array.array('B', [
    DS4Button.L2,
    DS4Button.L1,
    DS4Button.SHARE,
    255,    # Up
    255,    # Right
    255,    # Down
    255,    # Left
    DS4Button.L3,
    DS4Button.SHARE,
    222,
    222,
    222])

BUTTON_MAP_RIGHT = array.array('B', [
    DS4Button.R2,
    DS4Button.R1,
    DS4Button.OPTIONS,
    DS4Button.SQUARE,
    DS4Button.CIRCLE,
    DS4Button.TRIANGLE,
    DS4Button.CROSS,
    DS4Button.R3,
    DS4Button.LOGO,
    222,
    222,
    222])

BUTTONS_MAP_DPAD = array.array('B', [
    #                     LDRU
    DS4DPad.CENTERED,    # 0000
    DS4DPad.UP,          # 0001
    DS4DPad.RIGHT,       # 0010
    DS4DPad.UP_RIGHT,    # 0011
    DS4DPad.DOWN,        # 0100
    DS4DPad.CENTERED,    # 0101
    DS4DPad.DOWN_RIGHT,  # 0110
    DS4DPad.CENTERED,    # 0111
    DS4DPad.LEFT,        # 1000
    DS4DPad.UP_LEFT,     # 1001
    DS4DPad.CENTERED,    # 1010
    DS4DPad.CENTERED,    # 1011
    DS4DPad.DOWN_LEFT,   # 1100
    DS4DPad.CENTERED,    # 1101
    DS4DPad.CENTERED,    # 1110
    DS4DPad.CENTERED     # 1111
])

def read_js_left():
    dpad_bits = 0
    while True:
        evbuf_left = js_left.read(8)
        if evbuf_left:
            time, value, type, number = unpack('IhBB', evbuf_left)
            if type & 0x01: # button event
                button_out = BUTTON_MAP_LEFT[number]
                if button_out == 255:
                    if value:
                        dpad_bits |= (1 << (number - 3))
                    else:
                        dpad_bits &= ~(1 << (number - 3))
                    ds4g.dPad(BUTTONS_MAP_DPAD[dpad_bits])
                else:
                    if value:
                        ds4g.press(button_out)
                    else:
                        ds4g.release(button_out)

            if type & 0x02: # axis event
                # DS4 wants values 0..128..255 where 128 is center position
                axis = ((value + 32767) >> 8)
                if axis == 127:
                    axis = 128
                # Axes 0,1 left stick X,Y
                if number == 0:
                    ds4g.leftXAxis(axis)
                elif number == 1:
                    ds4g.leftYAxis(axis)

def read_js_right():
    while True:
        evbuf_right = js_right.read(8)
        if evbuf_right:
            time, value, type, number = unpack('IhBB', evbuf_right)
            if type & 0x01: # button event
                button_out = BUTTON_MAP_RIGHT[number]
                if value:
                    ds4g.press(button_out)
                else:
                    ds4g.release(button_out)

            if type & 0x02: # axis event
                # DS4 wants values 0..128..255 where 128 is center position
                axis = ((value + 32767) >> 8)
                if axis == 127:
                    axis = 128
                # Axes 0,1 left stick X,Y
                if number == 0:
                    ds4g.rightXAxis(axis)
                elif number == 1:
                    ds4g.rightYAxis(axis)


while True:
    task_left = threading.Thread(target=read_js_left)
    task_right = threading.Thread(target=read_js_right)
    task_left.start()
    task_right.start()

    while True:
        time.sleep(60)
