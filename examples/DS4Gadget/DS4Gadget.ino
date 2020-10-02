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
 * Sony Dual Shock 4 Gadget
 *
 * Emulate a Sony DS4 compatible USB Gamepad on the USB port.
 * Receive gamepad event input on Serial1 from another computer.
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

#include "HID-Project.h"  // https://github.com/NicoHood/HID

// On SAMD boards where the native USB port is also the serial console, use
// Serial1 for the serial console. This applies to all SAMD boards except for
// Arduino Zero and M0 boards.
#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAM_ZERO)
#define SerialDebug SERIAL_PORT_MONITOR
#else
#define SerialDebug Serial1
#endif

// Maximum safe baud rate for Teeny LC
//#define GADGET_BPS  500000
// Maximum safe baud rate for SAMD M0
#define GADGET_BPS  2000000

#define GADGET_UART    Serial1
#define gadget_begin(...)      GADGET_UART.begin(__VA_ARGS__)
#define gadget_print(...)      GADGET_UART.print(__VA_ARGS__)
#define gadget_write(...)      GADGET_UART.write(__VA_ARGS__)
#define gadget_println(...)    GADGET_UART.println(__VA_ARGS__)
#define gadget_read(...)       GADGET_UART.read(__VA_ARGS__)
#define gadget_readBytes(...)  GADGET_UART.readBytes(__VA_ARGS__)
#define gadget_available(...)  GADGET_UART.available(__VA_ARGS__)
#define gadget_setTimeout(...) GADGET_UART.setTimeout(__VA_ARGS__)

#define DEBUG_ON  0
#if DEBUG_ON
#define dbbegin(...)      SerialDebug.begin(__VA_ARGS__)
#define dbprint(...)      SerialDebug.print(__VA_ARGS__)
#define dbprintln(...)    SerialDebug.println(__VA_ARGS__)
#define dbwrite(...)      SerialDebug.write(__VA_ARGS__)
#else
#define dbbegin(...)
#define dbprint(...)
#define dbprintln(...)
#define dbwrite(...)
#endif

uint32_t elapsed_mSecs(uint32_t last_millis)
{
  uint32_t now = millis();
  if (now < last_millis) {
    return (now + 1) + (0xFFFFFFFF - last_millis);
  }
  else {
    return now - last_millis;
  }
}

const uint8_t STX = 0x02;
const uint8_t ETX = 0x03;

uint8_t gadget_report(uint8_t *buffer, size_t buflen)
{
  static uint8_t gadget_buffer[128];
  static uint8_t gadget_buflen;
  static uint8_t gadget_state=0;
  static uint8_t gadget_expectedlen;
  static uint32_t timeout_ms;
  size_t bytesIn;

  int byt;

  while (gadget_available() > 0) {
    dbprint("a="); dbprintln(gadget_available());
    switch (gadget_state) {
      case 0:
        dbprint(gadget_state); dbprint(',');
        byt = gadget_read();
        if (byt != -1) {
          dbprintln(byt, HEX);
          if (byt == STX) {
            timeout_ms = millis();
            gadget_state = 1;
            gadget_buflen = 0;
          }
        }
        break;
      case 1:
        dbprint(gadget_state); dbprint(',');
        byt = gadget_read();
        if (byt != -1) {
          dbprintln(byt, HEX);
          gadget_buffer[0] = byt;
          gadget_buflen = 1;
          gadget_expectedlen = byt;
          if (gadget_expectedlen > (sizeof(gadget_buffer) - 1)) {
            gadget_expectedlen = sizeof(gadget_buffer) - 1;
          }
          gadget_state = 2;
        }
        break;
      case 2:
        dbprint(gadget_state); dbprint(',');
        byt = gadget_read();
        if (byt != -1) {
          dbprintln(byt, HEX);
          gadget_buffer[1] = byt;
          gadget_buflen = 2;
          gadget_state = 3;
        }
        break;
      case 3:
        dbprint(gadget_state); dbprint(',');
        bytesIn = gadget_readBytes(&gadget_buffer[gadget_buflen], gadget_expectedlen - gadget_buflen + 1);
        dbprintln(bytesIn);
        if (bytesIn > 0) {
          gadget_buflen += bytesIn;
          if (gadget_buflen > gadget_expectedlen) {
            gadget_state = 4;
          }
        }
        break;
      case 4:
        dbprint(gadget_state); dbprint(',');
        byt = gadget_read();
        if (byt != -1) {
          dbprintln(byt, HEX);
          if (byt == ETX) {
            if (gadget_buflen > buflen) gadget_buflen = buflen;
            memcpy(buffer, gadget_buffer, gadget_buflen);
            gadget_state = 0;
            return gadget_buflen;
          }
          else if (byt == STX) {
            timeout_ms = millis();
            gadget_state = 1;
            gadget_buflen = 0;
            return 0;
          }
          gadget_state = 0;
        }
        break;
      default:
        dbprintln("gadget_read: invalid state");
        break;
    }
  }
  // If STX seen and more than 2 ms, give up and go back to looking for STX
  if ((gadget_state != 0) && (elapsed_mSecs(timeout_ms) > 2)) {
    gadget_state = 0;
    static uint32_t timeout_error = 0;
    timeout_error++;
    digitalWrite(LED_BUILTIN, timeout_error & 1);
  }
  return 0;
}

void setup()
{
  // Turn off built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  gadget_begin( GADGET_BPS );
  gadget_setTimeout(0);
#if SerialDebug != GADGET_UART
  dbbegin( 115200 );
#endif
  dbprintln("DS4Gadget setup");

#if HAS_DOTSTAR_LED
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif

  DS4Gamepad.begin();
}

void loop()
{
  uint8_t gadget_data[128];
  memset(gadget_data, 0, sizeof(gadget_data));
  uint8_t reportLen = gadget_report(gadget_data, sizeof(gadget_data));
  if ((reportLen > 1) && (gadget_data[1] == 3)) {
    DS4Gamepad.write(&gadget_data[2]);
  }

  DS4Gamepad.loop();
}
