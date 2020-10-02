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

// Include guard
#pragma once

DS4GamepadAPI::DS4GamepadAPI(void)
{
    // Empty
}

void DS4GamepadAPI::begin(void) {
    // release all buttons
    memset(&_report, 0, sizeof(_report));
    end();
    startMillis = millis();
}

void DS4GamepadAPI::loop(void) {
    uint32_t endMillis = millis();
    uint32_t deltaMillis;
    if (endMillis >= startMillis) {
        deltaMillis = (endMillis - startMillis);
    }
    else {
        deltaMillis = ((0xFFFFFFFFUL - startMillis) + endMillis);
    }
    if (deltaMillis >= 3) {
        write();
        startMillis = millis();
    }
}

void DS4GamepadAPI::end(void) {
    releaseAll();
    write();
}

void DS4GamepadAPI::write(void) {
    SendReport(&_report, sizeof(_report));
    _report.reportCnt++;
    _report.timestamp += 188;
}


void DS4GamepadAPI::write(void *report) {
    uint16_t save_timestamp = _report.timestamp;
    uint8_t save_reportCnt = _report.reportCnt;
    memcpy(&_report, report, sizeof(_report));
    _report.timestamp = save_timestamp;
    _report.reportCnt = save_reportCnt;
    write();
}


void DS4GamepadAPI::press(uint8_t b) {
    switch (b) {
        // button 1
        case DS4Button_SQUARE:
        case DS4Button_CROSS:
        case DS4Button_CIRCLE:
        case DS4Button_TRIANGLE:
            _report.button1 |= 1 << b;
            break;
            // button2
        case DS4Button_L1:
        case DS4Button_R1:
        case DS4Button_L2: // also axis
        case DS4Button_R2: // also axis
        case DS4Button_SHARE:
        case DS4Button_OPTIONS:
        case DS4Button_L3: // stick button
        case DS4Button_R3: // stick button
            _report.button2 |= 1 << (b - DS4Button_L1);
            break;
            // button3
        case DS4Button_LOGO:
        case DS4Button_TPAD:
            _report.button3 |= 1 << (b - DS4Button_LOGO);
            break;
        default:
            break;
    }
}


void DS4GamepadAPI::release(uint8_t b) {
    switch (b) {
        // button 1
        case DS4Button_SQUARE:
        case DS4Button_CROSS:
        case DS4Button_CIRCLE:
        case DS4Button_TRIANGLE:
            _report.button1 &= ~(1 << b);
            break;
            // button2
        case DS4Button_L1:
        case DS4Button_R1:
        case DS4Button_L2: // also axis
        case DS4Button_R2: // also axis
        case DS4Button_SHARE:
        case DS4Button_OPTIONS:
        case DS4Button_L3: // stick button
        case DS4Button_R3: // stick button
            _report.button2 &= ~(1 << (b - DS4Button_L1));
            break;
            // button3
        case DS4Button_LOGO:
        case DS4Button_TPAD:
            _report.button3 &= ~(1 << (b - DS4Button_LOGO));
            break;
        default:
            break;
    }
}


void DS4GamepadAPI::releaseAll(void) {
    uint16_t save_timestamp = _report.timestamp;
    uint8_t save_reportCnt = _report.reportCnt;
    memset(&_report, 0x00, sizeof(_report));
    _report.timestamp = save_timestamp;
    _report.reportCnt = save_reportCnt;
    _report.ReportID = 0x01;
    _report.leftXAxis = _report.leftYAxis = 0x80;
    _report.rightXAxis = _report.rightYAxis = 0x80;
    _report.dPad = DS4GAMEPAD_DPAD_CENTERED;
    //memset does this
    //_report.button1 = _report.button2 = _report.button3 = 0;
    //_report.L2Axis = _report.R2Axis = 0;
}

void DS4GamepadAPI::buttons(uint16_t b) {
    _report.button1 = _report.button2 = _report.button3 = 0;
    for (int i = DS4Button_SQUARE; i <= DS4Button_TPAD; i++) {
        if (b & (1 << i)) press(i);
    }
}


void DS4GamepadAPI::leftXAxis(uint8_t a) {
    _report.leftXAxis = a;
}


void DS4GamepadAPI::leftYAxis(uint8_t a) {
    _report.leftYAxis = a;
}


void DS4GamepadAPI::rightXAxis(uint8_t a) {
    _report.rightXAxis = a;
}


void DS4GamepadAPI::rightYAxis(uint8_t a) {
    _report.rightYAxis = a;
}


void DS4GamepadAPI::rightTrigger(uint8_t a) {
    _report.R2Axis = a;
};


void DS4GamepadAPI::leftTrigger(uint8_t a) {
    _report.L2Axis = a;
};


void DS4GamepadAPI::dPad(int8_t d) {
    _report.dPad = d;
}
