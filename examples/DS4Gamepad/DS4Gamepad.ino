/*
 * MIT License
 *
 * Copyright (c) 2020 gdsports625@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
   Basic (sticks and buttons) gamepad controller for Dual Shock 4

   The gamepad has 14 buttons, 2 analog thumbsticks, 1 8-way direction pad,
   and 2 analog triggers.
   Compared to the Dual Shock 4 controller, the DS4Gamepad does
   not have rumble motors, motion sensors (accel/gyro), Bluetooth,
   touch pad, and light panel.

## Adafruit ItsyBitsy M0/M4 Express

DS4 Button  |Button Name    |ItsyBitsy Pin
------------|---------------|-------------
0           |SQUARE         |0
1           |CROSS          |1
2           |CIRCLE         |2
3           |TRIANGLE       |3
4           |L1             |4
5           |R1             |7
6           |L2*            |-
7           |R2*            |-
8           |SHARE          |9
9           |OPTIONS        |10
10          |L3#            |11
11          |R3#            |12
12          |LOGO$          |13
13          |TPAD%          |SCK

* L2/R2 = Analog triggers are also reported as buttons but do not need input pins.
# L3/R3 = Thumbsticks are also buttons.
$ LOGO = Pin 13 has resistor and LED pulling the pin to ground so it does not work as an input.
         Remove the resistor and/or LED to make this work.
% TPAD = touchpad is also a button.

DS4 Axis        |ItsyBitsy Pin
----------------|-------------
Left stick X    |A0
Left stick Y    |A1
Right stick X   |A2
Right stick Y   |A3
Left trigger    |A4
Right trigger   |A5

Direction pad requires 4 pins.

MOSI  button 1
MISO  button 0
SCL   button 2
SDA   button 3

*/

#define HAS_DOTSTAR_LED (defined(ADAFRUIT_TRINKET_M0) || defined(ADAFRUIT_ITSYBITSY_M0) || defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS))
#if HAS_DOTSTAR_LED
#include <Adafruit_DotStar.h>
#if defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS)
#define DATAPIN    8
#define CLOCKPIN   6
#elif defined(ADAFRUIT_ITSYBITSY_M0)
#define DATAPIN    41
#define CLOCKPIN   40
#elif defined(ADAFRUIT_TRINKET_M0)
#define DATAPIN    7
#define CLOCKPIN   8
#endif
Adafruit_DotStar strip = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
#endif

#include "HID-Project.h"
#include <Bounce2.h>

// BEWARE: Make sure all these pins are available on your board. Some pins
// may be connected to on board devices so cannot be used as inputs.
#define NUM_BUTTONS 12
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {0, 1, 2, 3, 4, 7, 9, 10, 11, 12, 13, SCK};
#define NUM_DPAD 4
const uint8_t DPAD_PINS[NUM_DPAD] = {MOSI, MISO, SCL, SDA};

// Translate 4 input pins to dpad direction values
const uint8_t DPAD_MAP[16] = {
                             // LDRU
  DS4GAMEPAD_DPAD_CENTERED,  // 0000 All dpad buttons up
  DS4GAMEPAD_DPAD_UP,        // 0001 direction UP
  DS4GAMEPAD_DPAD_RIGHT,     // 0010 direction RIGHT
  DS4GAMEPAD_DPAD_UP_RIGHT,  // 0011 direction UP RIGHT
  DS4GAMEPAD_DPAD_DOWN,      // 0100 direction DOWN
  DS4GAMEPAD_DPAD_CENTERED,  // 0101 invalid
  DS4GAMEPAD_DPAD_DOWN_RIGHT,// 0110 direction DOWN RIGHT
  DS4GAMEPAD_DPAD_CENTERED,  // 0111 invalid
  DS4GAMEPAD_DPAD_LEFT,      // 1000 direction LEFT
  DS4GAMEPAD_DPAD_UP_LEFT,   // 1001 direction UP LEFT
  DS4GAMEPAD_DPAD_CENTERED,  // 1010 invalid
  DS4GAMEPAD_DPAD_CENTERED,  // 1011 invalid
  DS4GAMEPAD_DPAD_DOWN_LEFT, // 1100 direction DOWN LEFT
  DS4GAMEPAD_DPAD_CENTERED,  // 1101 invalid
  DS4GAMEPAD_DPAD_CENTERED,  // 1110 invalid
  DS4GAMEPAD_DPAD_CENTERED,  // 1111 invalid
};

Bounce * buttons = new Bounce[NUM_BUTTONS];
Bounce * dpad = new Bounce[NUM_DPAD];

uint8_t dpad_bits = 0;

typedef struct axis_t {
  uint16_t adcMin;
  uint16_t adcMax;
  const uint8_t toMin;
  const uint8_t toMax;
} axis_t;

// Set the initial min/max 1/8 away from the ADC min/max.
#define ARDUINO_ADC_RESOLUTION  (10)
#define ADC_INITIAL_MIN (1 << (ARDUINO_ADC_RESOLUTION - 3))
#define ADC_INITIAL_MAX ((1 << ARDUINO_ADC_RESOLUTION) - ADC_INITIAL_MIN)

axis_t LeftX = {
  ADC_INITIAL_MIN, ADC_INITIAL_MAX, 0, 255
};
axis_t LeftY = {
  ADC_INITIAL_MIN, ADC_INITIAL_MAX, 255, 0
};
axis_t RightX = {
  ADC_INITIAL_MIN, ADC_INITIAL_MAX, 0, 255
};
axis_t RightY = {
  ADC_INITIAL_MIN, ADC_INITIAL_MAX, 255, 0
};
axis_t LeftTrigger = {
  ADC_INITIAL_MIN, ADC_INITIAL_MAX, 0, 255
};
axis_t RightTrigger = {
  ADC_INITIAL_MIN, ADC_INITIAL_MAX, 0, 255
};

// Dynamically determine the pot/ADC limits because my craptastic
// analog sticks have dead zones at the ends.
uint8_t axisRead(int analogPin, struct axis_t &ax)
{
  uint16_t x = analogRead(analogPin);
  if (x > ax.adcMax) ax.adcMax = x;
  if (x < ax.adcMin) ax.adcMin = x;
  return map(x, ax.adcMin, ax.adcMax, ax.toMin, ax.toMax);
}

void setup() {
  // Pin 13 has resistor and LED pulling the pin to ground so it does not work as an
  // input. Remove the resistor and/or LED to make pin 13/touch pad button work.
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i], INPUT_PULLUP );
  }
  for (int i = 0; i < NUM_DPAD; i++) {
    dpad[i].attach( DPAD_PINS[i] , INPUT_PULLUP  );
  }

#if HAS_DOTSTAR_LED
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif

  // Sends a clean HID report to the host.
  DS4Gamepad.begin();
}

void loop() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    // Update the Bounce instance :
    buttons[i].update();
    uint8_t button_number = (i > 5) ? i + 2: i;
    // Button fell means button pressed
    if ( buttons[i].fell() ) {
      DS4Gamepad.press(button_number);
    }
    else if ( buttons[i].rose() ) {
      DS4Gamepad.release(button_number);
    }
  }
  for (int i = 0; i < NUM_DPAD; i++) {
    // Update the Bounce instance
    dpad[i].update();
    // Button fell means button pressed
    if ( dpad[i].fell() ) {
      dpad_bits |= (1 << i);
    }
    else if ( dpad[i].rose() ) {
      dpad_bits &= ~(1 << i);
    }
  }
  DS4Gamepad.dPad(DPAD_MAP[dpad_bits]);

  DS4Gamepad.rightXAxis(axisRead(0, RightX));
  DS4Gamepad.rightYAxis(axisRead(1, RightY));
  DS4Gamepad.leftXAxis(axisRead(2, LeftX));
  DS4Gamepad.leftYAxis(axisRead(3, LeftY));

  uint8_t axis_temp = axisRead(4, LeftTrigger);
  DS4Gamepad.leftTrigger(axis_temp);
  if (axis_temp < 8)
    DS4Gamepad.release(DS4Button_L2);
  else
    DS4Gamepad.press(DS4Button_L2);

  axis_temp = axisRead(5, RightTrigger);
  DS4Gamepad.rightTrigger(axis_temp);
  if (axis_temp < 8)
    DS4Gamepad.release(DS4Button_R2);
  else
    DS4Gamepad.press(DS4Button_R2);

  DS4Gamepad.loop();
}
