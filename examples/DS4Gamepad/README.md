# PlayStation 4 Dual Shock 4 Gamepad Emulator

DS4Gamepad is gamepad software for the PlayStation 4 (PS4). It can be used to
build custom controllers or to create bots to automate boring parts of games.
The concept is not new and has been done with AVR 32u4, AVR 16u2, and Teensy
with the LUFA library. This project runs on SAMD processors and can be built
using the Arduino IDE.

DS4Gamepad not a hack or mod. It is open source USB gamepad software.
DS4Gamepad does NOT do controller authentication so when plugged into a PS4,
it will work for about 8 minutes. The MayFlash Magic-S Pro controller adapter
does do controller authentication so use it or something similar to get around
the 8 minute limit.

DS4Gamepad is built on NicoHood's HID library. In fact, it is a customized
version of the library Gamepad code. It is also an example of how to use the
Arduino PluggableUSB/PluggableHID/NicoHood HID library to emulate a real world
USB device.

Adafruit SAMD boards support capacitive touch so it should be possible to build
soft game controllers using conductive thread and conductive fabric. Also
Adafruit has SAMD51 (M4) boards running as fast as 200 MHz (overclocked).
Maybe TensorFlow Lite can do voice/word recognition and trigger button presses.

## Build this code in safe workspace

I highly recommend creating a safe Arduino IDE workspace for this project. If
this is not done, the Adafruit board modifications will affect all projects.

See the following page for background on the Arduino Portable IDE.

https://www.arduino.cc/en/Guide/PortableIDE

On an Ubuntu Linux system, the following creates a directory with a safe
Arduino IDE workspace.

```
$ tar xf ~/Downloads/arduino-1.8.13-linux64.tar.xz
$ cd arduino-1.8.13
$ mkdir portable
$ ./arduino &
```

The portable directory hold sketches, libraries, and board files. These
directories are separate from the ~/Arduino and ~/.arduino15 directories use in
normal mode.

In the Arduino IDE, go to File | Preferences then uncheck "Check for updates on
startup". This is optional but if not done, board updates may wipe out changes
made for this project.

Install the Arduino SAMD board package. Optionally, install the Adafruit
SAMD board package. See adafruit.com for details. If using an Adafruit board
both SAMD packages are required.

Use Library manager to install the Bounce2 library which is used by the
DS4Gamepad example. You could use a different debounce library or write you
own debounce code.

Changing the USB Vendor ID and Product ID is not needed.

### USB CDC ACM serial

The real gamepad does not have a USB CDC ACM port but DS4Gamepad does. The
serial port is also used to upload new sketches so removing it makes the board
harder to use. Remember new sketches always can be uploaded by double clicking the
board RESET button before starting the IDE upload.

For Adafruit SAMD boards, remove CDC descriptors by commenting
out "#define CDC_ENABLED" in USBDesc.h.

```
nano portable/packages/adafruit/hardware/samd/1.5.9/cores/arduino/USB/USBDesc.h
```

```
// CDC or HID can be enabled together.
//#define CDC_ENABLED
#define PLUGGABLE_USB_ENABLED

#ifdef CDC_ENABLED
```

### USB Device Descriptors

The class, subclass, and protocol are different even after the CDC port is
removed. This can be fixed by changing source files in the board package.

For Arduino SAMD boards, bracket two lines of code in USBCore.cpp with #ifdef CDC_ENABLED/#endif.

```
nano portable/packages/arduino/hardware/samd/1.8.4/cores/arduino/USB/USBCore.cpp
```

```
	{
#ifdef CDC_ENABLED
		if (setup.wLength == 8)
			_cdcComposite = 1;
#endif

		desc_addr = _cdcComposite ?  (const uint8_t*)&USB_DeviceDescriptorB : (const uint8_t*)&USB_DeviceDescriptor;
```

For Adafruit SAMD boards, do the same thing but on a different version of the file.

```
nano portable/packages/adafruit/hardware/samd/1.5.9/cores/arduino/USB/USBCore.cpp
```

### OUT Endpoint

The real gamepad has an OUT as well as an IN endpoint. I am guessing the OUT
endpoint is intended for rumble control but since the controller does not have
rumble motors nothing is sent to the endpoint.

