
// Reference design for usage of a rotary encoder. 
// See https://github.com/nicolacimmino/RotaryEncoder for detailed explanation.
//  Copyright (C) 2018 Nicola Cimmino
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
#include <PinChangeInterrupt.h>

#define PIN_ENC_A 10
#define PIN_ENC_B 11
#define PIN_ENC_SW 12

#define DISPLAY_I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

volatile int16_t counter = 0;
volatile byte encoderMode = 0;
volatile byte encoderDynamicMode = 0;

/**
 * Array of function pointers to setup the various modes.
 */
extern void setup1XEncode();
extern void setup2XEncode();
extern void setup4XEncode();

void (*modesSetters[3])() = {setup1XEncode, setup2XEncode, setup4XEncode};

/**
 * Labels for the mode headers.
 */
String modeHeaders[3] = {"1X    ", "  2X  ", "    4X"};
String dynamicModeHeaders[2] = {"  LIN", "     EXP"};

/**
 * Change mode. 
 * 
 * Change mode is entered when pressing the encoder switch. A short press will change the encode mode while
 * a long one will change the encoder dynamic mode.
 * 
 */
void changeMode()
{
    // Allow the switch to stabilise.
    byte debounce = 0x55;
    while (debounce != 0x00)
    {
        debounce = (debounce < 1) | (digitalRead(PIN_ENC_SW) & 1);
        delay(1);
    }

    // Wait for the switch to be released or a timeout of 500mS to expire.
    unsigned long initialTime = millis();
    while ((millis() - initialTime < 500) && digitalRead(PIN_ENC_SW) == 0)
    {
        delay(1);
    }

    if (digitalRead(PIN_ENC_SW) == 0)
    {
        encoderDynamicMode = (encoderDynamicMode + 1) % 2;
    }
    else
    {
        encoderMode = (encoderMode + 1) % (sizeof(modesSetters) / sizeof(modesSetters[0]));
        modesSetters[encoderMode]();
    }

    // Reflect immedidately on screen the action taken.
    printHeader();

    // Ensure  the switch has been released or we will trigger an
    // extra mode change as we will re-enter this function.
    while (digitalRead(PIN_ENC_SW) == 0)
    {
        delay(1);
    }
}

/**
 * Apply a change to the counter. This is invoked at every step detected.
 * In liear mode this is a fix increment of 1 at every step. In dynamic
 * mode the speed of rotation affects the amount of steps.
 */
void applyCounterChange(bool cw)
{
    int factor = 0;
    if (encoderDynamicMode == 0)
    {
        factor = 1;
    }
    else if (encoderDynamicMode == 1)
    {
        static unsigned long lastDetentTime = 0;
        factor = min(5, max(1, (20 - (signed long)(millis() - lastDetentTime)) / 4));
        lastDetentTime = millis();
    }

    counter += cw ? factor : -factor;

    counter = (counter < 0) ? (360 + counter) : counter % 360;
}

/**
 * Print the header contiaining the modes flags indicators.
 */
void printHeader()
{
    oled.setCursor(0, 0);
    oled.set1X();
    oled.print(modeHeaders[encoderMode]);
    oled.print(dynamicModeHeaders[encoderDynamicMode]);
    oled.clearToEOL();
}

void setup()
{
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    oled.begin(&Adafruit128x64, DISPLAY_I2C_ADDRESS);
    oled.setFont(System5x7);

    modesSetters[encoderMode]();
    printHeader();
}

void loop()
{
    if (digitalRead(PIN_ENC_SW) == 0)
    {
        changeMode();
    }

    oled.setCursor(0, 3);
    oled.set2X();
    String counterString = String("000");
    counterString.concat(String(counter));
    counterString = counterString.substring(counterString.length() - 3);
    oled.print(counterString);
}