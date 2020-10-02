# PlayStation 4 Dual Shock 4Gamepad Gadget

Emulate a PlayStation 4 Dual Shock 4 compatible USB Gamepad (Hori Mini4) on
the USB port. Receive gamepad event input on Serial1 from another computer.
Works on Adafruit Trinket M0. The Hori Mini 4 gamepad controller is licensed
by Sony.

The data format in the serial port looks like this.

```
<STX> <length> <type> 12 bytes of binary data <ETX>
0x02    0x0B    0x03                          0x03
```

```
struct ATTRIBUTE_PACKED {
    uint8_t	ReportID;   // always 0x01
    uint8_t	leftXAxis;
    uint8_t	leftYAxis;
    uint8_t	rightXAxis;
    uint8_t	rightYAxis;
    uint8_t dPad:4;     // dpad[3-0]
    uint8_t button1:4;  // Triangle[7], circle[6], cross[5], square[4]
    uint8_t button2;    // R3:7,L3:6,Options:5,share:4,R2:3,L2:2,R1:1,L1:0
    uint8_t button3:2;  // tpad click[1], logo[0]
    uint8_t reportCnt:6;//
    uint8_t L2Axis;
    uint8_t R2Axis;
}
```

## Using the Gadget

To use the gadget with a computer such as a Raspberry Pi, connect the Trinket
M0 TX/RX to RX/TX of a CP2104 USB serial adapter.

[CP2104 breakout board](https://www.adafruit.com/product/3309)
[Trinket M0](https://www.adafruit.com/product/3500)

Trinket M0  |CP2104
------------|---------
Gnd         |GND
RX(3)       |TXD
Bat         |5V

Plugs the CP2104 into a Raspberry Pi. Plugs the Trinket M0 running DS4Gadget.ino
into a PlayStation 4.

* python/ds4gpadserial.py

Class that handles formatting data to send over USB serial to the Trinket M0.

* python/ds4gamepad_test.py

Example to exercise ds4gpadserial.py.

* python/ds4gamepad_sniffer.py

Example using ds4gpadserial.py to pass through gamepad events from a real
HoriPAD gamepad to the DS4Gadget emulator. The Pi becomes a Pi-in-the-middle
gamepad sniffer. Record and playback of gamepad events is possible but not
implemented.

```
    HoriPAD -> Raspberry Pi -> CP2104 -> Trinket M0 -> PlayStation 4
               ds4gamepad_                DS4Gadget
                 sniffer.py
```

* python/ds4gamepad_le3dp.py

Example using ds4gpadserial.py to use a Logitech Extreme 3D Pro flight stick
as a game controller.

```
    Logitech -> Raspberry Pi -> CP2104 -> Trinket M0 -> PlayStation 4
    Extreme     ds4gamepad_                DS4Gadget
    3D Pro        le3dp.py
```
