//
// Example for the usage of the rotary encoder library.
// See https://github.com/nicolacimmino/RotaryEncoder for detailed explanation.
//
//  Copyright (C) 2019 Nicola Cimmino
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <SSD1306AsciiAvrI2c.h>
#include "src/RotaryEncoder.h"

#define PIN_ENC_A 10
#define PIN_ENC_B 11
#define PIN_ENC_SW 12

byte encoderMode = 0;
byte encoderPositionChangeMode = 0;
RotaryEncoder rotaryEncoder;

#define DISPLAY_I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

/**
 * Labels for the mode headers.
 */
String modeHeaders[3] = {"1X    ", "  2X  ", "    4X"};
uint8_t modes[3] = {ROTARY_ENCODER_DECODE_MODE_1X, ROTARY_ENCODER_DECODE_MODE_2X, ROTARY_ENCODER_DECODE_MODE_4X};

/**
 * Labels for the position change mode headers.
 */
String positionChangeModeHeaders[2] = {"  LIN", "     DYN"};
uint8_t positionChangeModes[2] = {ROTARY_ENCODER_MODE_LINEAR, ROTARY_ENCODER_MODE_DYNAMIC};

void setup()
{
    oled.begin(&Adafruit128x64, DISPLAY_I2C_ADDRESS);
    oled.setFont(System5x7);

    rotaryEncoder.begin(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW, ROTARY_ENCODER_DECODE_MODE_2X, ROTARY_ENCODER_MODE_DYNAMIC, 0, 200);
    rotaryEncoder.registerOnClickCallback(click);
    rotaryEncoder.registerOnLongPressCallback(longPress);
    rotaryEncoder.registerOnRotationCallback(rotation);

    printHeader();
}

/*
 * Clicking on the switch moves to the next encoder mode.
 */
void click()
{
    encoderMode = (encoderMode + 1) % (sizeof(modes) / sizeof(modes[0]));
    rotaryEncoder.setDecodeMode(modes[encoderMode]);

    printHeader();
}

/*
 * A long press on the switch moves to the next position change mode.
 */
void longPress()
{
    encoderPositionChangeMode = (encoderPositionChangeMode + 1) % (sizeof(positionChangeModes) / sizeof(positionChangeModes[0]));
    rotaryEncoder.setPositionChangeMode(encoderPositionChangeMode);

    printHeader();
}

/*
 * A rotation of the encoder prints the new position on the screen.
 */
void rotation(bool cw, int position)
{
    oled.setCursor(0, 3);
    oled.set2X();
    oled.print(position);
    oled.clearToEOL();
}

/**
 * Print the header contiaining the modes flags indicators.
 */
void printHeader()
{
    oled.setCursor(0, 0);
    oled.set1X();
    oled.print(modeHeaders[encoderMode]);
    oled.print(positionChangeModeHeaders[encoderPositionChangeMode]);
    oled.clearToEOL();
}

void loop()
{
    rotaryEncoder.loop();
}