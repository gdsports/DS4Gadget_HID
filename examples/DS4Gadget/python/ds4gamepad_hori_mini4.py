#!/usr/bin/python3
"""
Read from Thrustmaster T.16000M FCS and write to DS4Gadget.

T16K -> Raspberry Pi -> DS4Gadget -> PlayStation 4
"""
import os
import time
from struct import unpack
import threading
import array
from fcntl import ioctl
import serial
from ds4gpadserial import DS4GamepadSerial, DS4Button

DS4G = DS4GamepadSerial()
DS4G.begin(serial.Serial('/dev/ttyAMA0', 2000000, timeout=0))

def read_hori_mini4(jsdev):
    """
    The Hori Mini4 is a licensed PS4 compatible controller. The throttles are
    analog (see axes) and binary (see buttons). Runs as a thread.

    axis    0: left stick X
            1: left stick Y
            2: right stick X
            3: left throttle
            4: right throttle
            5: right stick Y
            6: dPad X
            7: dPad Y

    button  0: cross
            1: circle
            2: triangle
            3: square
            4: L1
            5: R1
            6: L2
            7: R2
            8: share
            9: options
           10: logo
           11: L3
           12: R3


    share   options

            triangle
    square          circle
            cross
    """

    BUTTON_MAP = array.array('B', [
        DS4Button.SQUARE,
        DS4Button.CROSS,
        DS4Button.CIRCLE,
        DS4Button.TRIANGLE,
        DS4Button.L1,
        DS4Button.R1,
        DS4Button.L2,
        DS4Button.R2,
        DS4Button.SHARE,
        DS4Button.OPTIONS,
        DS4Button.L3,
        DS4Button.R3,
        DS4Button.LOGO,
        DS4Button.TPAD])

    while True:
        try:
            evbuf = jsdev.read(8)
        except:
            jsdev.close()
            break
        if evbuf:
            timestamp, value, type, number = unpack('IhBB', evbuf)
            if type == 0x01: # button event
                button_out = BUTTON_MAP[number]
                if value:
                    DS4G.press(button_out)
                else:
                    DS4G.release(button_out)

            if type == 0x02: # axis event
                axis = ((value + 32768) >> 8)
                # Axes 0,1 left stick X,Y
                if number == 0:
                    DS4G.leftXAxis(axis)
                elif number == 1:
                    DS4G.leftYAxis(axis)
                # Axes 2,3 right stick X,Y
                elif number == 2:
                    DS4G.rightXAxis(axis)
                elif number == 3:
                    DS4G.leftTrigger(axis)
                elif number == 4:
                    DS4G.rightTrigger(axis)
                elif number == 5:
                    DS4G.rightYAxis(axis)
                # Axes 6,7 directional pad X,Y
                elif number == 6:
                    DS4G.dPadXAxis(axis)
                elif number == 7:
                    DS4G.dPadYAxis(axis)

def read_ps4ds(jsdev):
    """
    The Sony PlayStation 4 controller has fewer buttons. The throttles are
    analog (see axes) and binary (see buttons). Runs as a thread.

    axis    0: left stick X
            1: left stick Y
            2: left throttle
            3: right stick X
            4: right stick Y
            5: right throttle
            6: dPad X
            7: dPad Y

    button  0: cross            NS B
            1: circle           NS A
            2: triangle         NS X
            3: square           NS Y
            4: left trigger     NS left trigger
            5: right trigger    NS right trigger
            6: left throttle    NS left throttle
            7: right throttle   NS right throttle
            8: share            NS minus
            9: options          NS plus
           10: logo             NS home
           11: left stick button  NS left stick button
           12: right stick button NS rgith stick button


    share   options

            triangle
    square          circle
            cross
    """

    BUTTON_MAP = array.array('B', [
        DS4Button.SQUARE,
        DS4Button.CROSS,
        DS4Button.CIRCLE,
        DS4Button.TRIANGLE,
        DS4Button.L1,
        DS4Button.R1,
        DS4Button.L2,
        DS4Button.R2,
        DS4Button.SHARE,
        DS4Button.OPTIONS,
        DS4Button.L3,
        DS4Button.R3,
        DS4Button.LOGO,
        DS4Button.TPAD])

    while True:
        try:
            evbuf = jsdev.read(8)
        except:
            jsdev.close()
            break
        if evbuf:
            timestamp, value, type, number = unpack('IhBB', evbuf)
            if type == 0x01: # button event
                button_out = BUTTON_MAP[number]
                if value:
                    DS4G.press(button_out)
                else:
                    DS4G.release(button_out)

            if type == 0x02: # axis event
                axis = ((value + 32768) >> 8)
                # Axes 0,1 left stick X,Y
                if number == 0:
                    DS4G.leftXAxis(axis)
                elif number == 1:
                    DS4G.leftYAxis(axis)
                elif number == 2:
                    DS4G.leftTrigger(axis)
                # Axes 3,4 right stick X,Y
                elif number == 3:
                    DS4G.rightXAxis(axis)
                elif number == 4:
                    DS4G.rightYAxis(axis)
                elif number == 5:
                    DS4G.rightTrigger(axis)
                # Axes 6,7 directional pad X,Y
                elif number == 6:
                    DS4G.dPadXAxis(axis)
                elif number == 7:
                    DS4G.dPadYAxis(axis)


def main():
    joysticks = {}
    joysticks_to_del = list()
    while True:
        # For all known joysticks make sure its thread is still alive. If not, forget the joystick
        # The thread ends when the joystick is unplugged.
        for jsname in joysticks:
            if not joysticks[jsname].is_alive():
                print("joystick %s removed" % jsname)
                joysticks_to_del.append(jsname)
        for jsname in joysticks_to_del:
            del joysticks[jsname]
        joysticks_to_del.clear()
        # For all joysticks in /dev/input/ and not do not have a thread, start a thread
        # Each thread reads from its associated joystick and writes to the NS gadget device.
        for fn in os.listdir('/dev/input'):
            if fn.startswith('js'):
                jsname = '/dev/input/' + fn
                if not jsname in joysticks:
                    try:
                        jsdev = open(jsname, 'rb')
                    except:
                        break
                    buf = array.array('B', [0] * 64)
                    ioctl(jsdev, 0x80006a13 + (0x10000 * len(buf)), buf) # JSIOCGNAME(len)
                    jslongname = buf.tobytes().rstrip(b'\x00').decode('utf-8').upper()
                    if 'HORI CO.,LTD. HORIPAD MINI4' in jslongname:
                        print("Found Hori Mini4 licensed by Sony")
                        thr_id = threading.Thread(target=read_hori_mini4, args=(jsdev,), daemon=True)
                        thr_id.start()
                        joysticks[jsname] = thr_id
                    elif 'SONY INTERACTIVE ENTERTAINMENT WIRELESS CONTROLLER' in jslongname:
                        print("Found Sony Dual Shock")
                        thr_id = threading.Thread(target=read_ps4ds, args=(jsdev,), daemon=True)
                        thr_id.start()
                        joysticks[jsname] = thr_id
                    else:
                        jsdev.close()

        time.sleep(0.1)

if __name__ == "__main__":
    main()
