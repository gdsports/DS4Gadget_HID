# PlayStation Dual Shock 4 Gamepad

Create PlayStation Dual Shock 4 compatible gamepads using Adafruit SAMD
boards. This project depends on the NicoHood Arduino HID library.

This project works on Adafruit SAMD boards but has not been tested in AVR
boards and other SAMD boards.

This project is not a library. The code in the src directory must be copied
into the HID-Project library. Be sure to install the latest version of
HID-Project from github. The version installed using Library Manager is too
old.

Copy the source code files in the src directory to directories with the same
names in HID-Project library installation directory. See DS4Gamepad/README.md
for details.

The Adafruit SAMD board package version 1.5.14 must be used for Adafruit SAMD
boards. Version 1.6.0 and newer do not work with this project.

If using a Linux system, acli.sh uses (Arduino
CLI)[https://arduino.github.io/arduino-cli/latest/] to build the code. It also
applies patches required to make DS4Gadget compatible as possible with a real DS4
gamepad. acli.sh creates the build directory in /tmp to avoid changing the
default Arduino directories. This is done because patches to SAMD board files
are required.

There are two binary (UF2) versions for each example. For example, for DS4Gadget
the two versions are shown below.

```
./examples/DS4Gadget/firmware/vidds4/adafruit.samd.adafruit_trinket_m0/DS4Gadget.ino.bin.uf2
./examples/DS4Gadget/firmware/viddefault/adafruit.samd.adafruit_trinket_m0/DS4Gadget.ino.bin.uf2
```

The viddefault version uses the default Adafruit USB VID and PID. This works
with the MayFlash Magic-S Pro controller adapter connected to a Playstation 4.

The vidds4 version uses the DS4 USB VID and PID. This version works with
https://gamepadviewer.com/ when connected to a PC. gamepadviewer cannot
identify the gamepad without the DS4 VID and PID. This version does NOT
work with the MayFlash Magic-S Pro.
