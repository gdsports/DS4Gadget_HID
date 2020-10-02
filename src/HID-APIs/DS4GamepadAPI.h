/*
Copyright (c) 2014-2015 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
 * This file is a version of GamepadAPI.h customized to make it
 * compatible with the Sony Dual Shock 4
 */

// Include guard
#pragma once

#include <Arduino.h>
#include "HID-Settings.h"

// Dpad directions
#define DS4GAMEPAD_DPAD_CENTERED 8
#define DS4GAMEPAD_DPAD_UP 0
#define DS4GAMEPAD_DPAD_UP_RIGHT 1
#define DS4GAMEPAD_DPAD_RIGHT 2
#define DS4GAMEPAD_DPAD_DOWN_RIGHT 3
#define DS4GAMEPAD_DPAD_DOWN 4
#define DS4GAMEPAD_DPAD_DOWN_LEFT 5
#define DS4GAMEPAD_DPAD_LEFT 6
#define DS4GAMEPAD_DPAD_UP_LEFT 7


enum DS4Buttons {
    // button1
    DS4Button_SQUARE = 0,
    DS4Button_CROSS,
    DS4Button_CIRCLE,
    DS4Button_TRIANGLE,
    // button2
    DS4Button_L1,
    DS4Button_R1,
    DS4Button_L2, // also axis
    DS4Button_R2, // also axis
    DS4Button_SHARE,
    DS4Button_OPTIONS,
    DS4Button_L3, // stick button
    DS4Button_R3, // stick button
    // button3
    DS4Button_LOGO,
    DS4Button_TPAD
};

// 14 Buttons, 6 Axes, 1 D-Pad
typedef struct ATTRIBUTE_PACKED {
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
    uint16_t timestamp; // increment by 188 per report
    uint8_t batteryLvl;
    uint16_t gyroX;
    uint16_t gyroY;
    uint16_t gyroZ;
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
    uint8_t filler[39];
} HID_DS4GamepadReport_Data_t;

class DS4GamepadAPI {
    public:
        inline DS4GamepadAPI(void);

        inline void begin(void);
        inline void end(void);
        inline void loop(void);
        inline void write(void);
        inline void write(void *report);
        inline void press(uint8_t b);
        inline void release(uint8_t b);
        inline void releaseAll(void);

        inline void buttons(uint16_t b);
        inline void leftXAxis(uint8_t a);
        inline void leftYAxis(uint8_t a);
        inline void rightXAxis(uint8_t a);
        inline void rightYAxis(uint8_t a);
        inline void rightTrigger(uint8_t a);
        inline void leftTrigger(uint8_t a);
        inline void dPad(int8_t d);

        // Sending is public in the base class for advanced users.
        virtual void SendReport(void* data, int length) = 0;

    protected:
        HID_DS4GamepadReport_Data_t _report;
        uint32_t startMillis;
};

// Implementation is inline
#include "DS4GamepadAPI.hpp"
