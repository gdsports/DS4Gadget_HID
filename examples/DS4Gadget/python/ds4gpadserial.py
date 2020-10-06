#!/usr/bin/python3
"""
Interface to Dual Shock 4 Gamepad Gadget (DS4Gadget.ino) via serial port.

MIT License

Copyright (c) 2020 gdsports625@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""
from struct import pack
import array
import threading
from enum import IntEnum

# Direction pad names
class DS4DPad(IntEnum):
    """DS4DPad direction names"""
    CENTERED = 0x8
    UP = 0
    UP_RIGHT = 1
    RIGHT = 2
    DOWN_RIGHT = 3
    DOWN = 4
    DOWN_LEFT = 5
    LEFT = 6
    UP_LEFT = 7

# Button names
class DS4Button(IntEnum):
    """ DS4 Button names """
    SQUARE = 0
    CROSS = 1
    CIRCLE = 2
    TRIANGLE = 3
    L1 = 4
    R1 = 5
    L2 = 6
    R2 = 7
    SHARE = 8
    OPTIONS = 9
    L3 = 10
    R3 = 11
    LOGO = 12
    TPAD = 13

# dPad Button names
class DPadButton(IntEnum):
    """ DPad button names """
    UP = 254
    DOWN = 253
    LEFT = 252
    RIGHT = 251

class DS4GamepadSerial:
    """Dual Shock 4 Gamepad Serial Interface"""
    # pylint: disable=too-many-instance-attributes
    compass_dir_x = array.array('B', \
            [0, 0, 128, 255, 255, 255, 128, 0, \
            128, 128, 128, 128, 128, 128, 128, 128, 128])
    compass_dir_y = array.array('B', \
            [128, 255, 255, 255, 128, 0, 0, 0,\
            128, 128, 128, 128, 128, 128, 128, 128, 128])

    def __init__(self):
        self.thread_lock = threading.Lock()
        self.ser_port = 0
        self.left_x_axis = 128
        self.left_y_axis = 128
        self.right_x_axis = 128
        self.right_y_axis = 128
        self.left_trigger = 0
        self.right_trigger = 0
        self.my_buttons = 0
        self.d_pad = DS4DPad.CENTERED
        self.dpad_x_axis = 128
        self.dpad_y_axis = 128

    def begin(self, serial_port):
        """Start DS4Gamepad"""
        with self.thread_lock:
            self.ser_port = serial_port
            self.left_x_axis = 128
            self.left_y_axis = 128
            self.right_x_axis = 128
            self.right_y_axis = 128
            self.my_buttons = 0
            self.d_pad = 15
            self.dpad_x_axis = 128
            self.dpad_y_axis = 128
            self.write()
        return

    def end(self):
        """End DS4Gamepad"""
        self.ser_port.close()
        return

    def write(self):
        """Send DS4Gamepad state"""
        self.ser_port.write(
            pack('<BBBBBBBBBBBBBB',
                 2,  # STX
                 11, # data len + 1
                 3,  # report type
                 1,  # report ID
                 self.left_x_axis, self.left_y_axis,
                 self.right_x_axis, self.right_y_axis,
                 ((self.my_buttons & 0x0f) << 4) | self.d_pad,
                 (self.my_buttons >> 4) & 0xff,
                 self.my_buttons >> 12,
                 self.left_trigger,
                 self.right_trigger,
                 3)) # ETX
        return

    def press(self, button_number):
        """Press button 0..13"""
        with self.thread_lock:
            self.my_buttons |= (1<<button_number)
            self.write()
        return

    def release(self, button_number):
        """Release button 0..13"""
        with self.thread_lock:
            self.my_buttons &= ~(1<<button_number)
            self.write()
        return

    def releaseAll(self):
        """Release all buttons"""
        with self.thread_lock:
            self.my_buttons = 0
            self.write()
        return

    def buttons(self, buttons):
        """Set all buttons 0..13"""
        with self.thread_lock:
            self.my_buttons = buttons
            self.write()
        return

    def leftXAxis(self, position):
        """Move left stick X axis 0..128..255"""
        with self.thread_lock:
            self.left_x_axis = position
            self.write()
        return

    def leftYAxis(self, position):
        """Move left stick Y axis 0..128..255"""
        with self.thread_lock:
            self.left_y_axis = position
            self.write()
        return

    def rightXAxis(self, position):
        """Move right stick X axis 0..128..255"""
        with self.thread_lock:
            self.right_x_axis = position
            self.write()
        return

    def rightYAxis(self, position):
        """Move right stick Y axis 0..128..255"""
        with self.thread_lock:
            self.right_y_axis = position
            self.write()
        return

    def allAxes(self, RYRXLYLX):
        """Change all axes from uint32_t."""
        with self.thread_lock:
            self.right_y_axis = (RYRXLYLX >> 24) & 0xFF;
            self.right_x_axis = (RYRXLYLX >> 16) & 0xFF;
            self.left_y_axis  = (RYRXLYLX >>  8) & 0xFF;
            self.left_x_axis  = (RYRXLYLX      ) & 0xFF;
            self.write()
        return

    def leftTrigger(self, position):
        """Move left trigger 0..255"""
        with self.thread_lock:
            self.left_trigger = position
            self.write()
        return

    def rightTrigger(self, position):
        """Move right trigger 0..255"""
        with self.thread_lock:
            self.right_trigger = position
            self.write()
        return

    def map_dpad_xy(self, x, y):
        """Return direction pad number given axes x,y"""
        if x == 128:
            if y == 128:
                return 15   # Center
            elif y < 128:
                return 0    # North
            return 4        # South
        elif x < 128:
            if y == 128:
                return 6    # West
            elif y < 128:
                return 7    # North West
            return 5        # South West
        else:
            if y == 128:
                return 2    # East
            elif y < 128:
                return 1    # North East
            return 3        # South East

    def dPadXAxis(self, position):
        """Move right stick X axis 0..128..255"""
        if (position < 0 or position > 255):
            position = 128
        with self.thread_lock:
            self.dpad_x_axis = position
            self.d_pad = self.map_dpad_xy(self.dpad_x_axis, self.dpad_y_axis)
            self.write()
        return

    def dPadYAxis(self, position):
        """Move right stick Y axis 0..128..255"""
        if (position < 0 or position > 255):
            position = 128
        with self.thread_lock:
            self.dpad_y_axis = position
            self.d_pad = self.map_dpad_xy(self.dpad_x_axis, self.dpad_y_axis)
            self.write()
        return

    def dPad(self, position):
        """Move directional pad (0..7, 15)"""
        if position < 0 or position > 7:
            position = 15
        with self.thread_lock:
            self.d_pad = position
            self.dpad_x_axis = self.compass_dir_x[position]
            self.dpad_y_axis = self.compass_dir_y[position]
            self.write()
        return

def main():
    """ test DS4GamepadSerial class """
    import sys
    import serial
    import time

    ds4g = DS4GamepadSerial()
    try:
        ds4g.begin(serial.Serial('/dev/ttyAMA0', 2000000, timeout=0))
    except:
        print('Cannot open /dev/ttyUSB0')
        sys.exit(1)

    print('Serial port open')
    while True:
        # Press and hold every button 0..13
        for button in range(0, 14):
            ds4g.press(button)
            time.sleep(0.1)
        time.sleep(1)
        # Release all buttons
        ds4g.releaseAll()
        time.sleep(1)
        # Press all 14 buttons at the same time
        ds4g.buttons(0x3fff)
        time.sleep(1)
        # Release all buttons
        ds4g.releaseAll()
        time.sleep(1)
        # Move directional pad in all directions
        # 0 = North, 1 = North-East, 2 = East, etc.
        for direction in range(0, 8):
            ds4g.dPad(direction)
            time.sleep(0.5)
        # Move directional pad to center
        ds4g.dPad(DS4DPad.CENTERED)

if __name__ == "__main__":
    main()
