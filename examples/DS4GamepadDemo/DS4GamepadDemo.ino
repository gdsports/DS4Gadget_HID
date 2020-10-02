/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  DS4GamepadDemo example
  Press a button and demonstrate DS4Gamepad actions

  You can also use DS4Gamepad1,2,3 and 4 as single report.
  This will use 1 endpoint for each DS4Gamepad.
*/

#define HAS_DOTSTAR_LED (defined(ADAFRUIT_TRINKET_M0) || defined(ADAFRUIT_ITSYBITY_M0) || defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS))

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

const int pinLed = LED_BUILTIN;
const int pinButton = 2;

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT_PULLUP);

#if HAS_DOTSTAR_LED
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif

  // Sends a clean report to the host. This is important on any Arduino type.
  DS4Gamepad.begin();
}

void loop() {
  static uint8_t count = DS4Button_SQUARE;
  if (count > DS4Button_TPAD) {
    DS4Gamepad.releaseAll();
    count = DS4Button_SQUARE;
  }
  DS4Gamepad.press(count);
  count++;

  // Move x/y Axis to a new position (16bit)
  DS4Gamepad.leftXAxis(random(0xFF));
  DS4Gamepad.leftYAxis(random(0xFF));
  DS4Gamepad.rightXAxis(random(0xFF));
  DS4Gamepad.rightYAxis(random(0xFF));
  DS4Gamepad.leftTrigger(random(0xFF));
  DS4Gamepad.rightTrigger(random(0xFF));

  // Go through all dPad positions
  // values: 0-8 (0==centered)
  static uint8_t dpad = DS4GAMEPAD_DPAD_UP;
  DS4Gamepad.dPad(dpad++);
  if (dpad > DS4GAMEPAD_DPAD_UP_LEFT)
    dpad = DS4GAMEPAD_DPAD_UP;

  // Functions above only set the values.
  // This writes the report to the host.
  DS4Gamepad.loop();
}
