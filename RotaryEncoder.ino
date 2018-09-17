
#include <SSD1306AsciiAvrI2c.h>
#include <PinChangeInterrupt.h>

#define PIN_ENC_A 10
#define PIN_ENC_B 11
#define PIN_ENC_SW 12
#define DISPLAY_I2C_ADDRESS 0x3C

SSD1306AsciiAvrI2c oled;

int16_t counter = 0;

byte mode = 0;

/**
 * Simple scroll mode.
 * 
 * We register an interrupt on the rising edge of A and simply sample B and determine direction of 
 * roation based on the value of B.
 * 
 * - Prone to skipping on bad encoders with a lot of chatter.
 * - Response only every second detent as we process a single edge of A.
 * - Tedius for large changes.
 */

void setupSimpleScroll()
{
    printModeHeader("Simple Scroll");
    detachPCINT(digitalPinToPCINT(PIN_ENC_A));
    attachPCINT(digitalPinToPCINT(PIN_ENC_A), processSimpleScroll, RISING);
}

void processSimpleScroll()
{
    int factor = digitalRead(PIN_ENC_B) ? 1 : -1;
    applyCounterChange(factor);
}

/**
 * Dynamic scroll mode.
 * 
 * We register an interrupt on the rising and falling edge of A. We then determine the direction of rotation based on 
 * the value of B or ~B depending on A. We also mesure the time interval between detents and smoothly increase the 
 * counter increase factor per detent based on the rotation speed.
 * 
 * - Reacts at every detent.
 * - Easier to make large changes.
 */

void setupDynamicScroll()
{
    printModeHeader("Dynamic Scroll");
    detachPCINT(digitalPinToPCINT(PIN_ENC_A));
    attachPCINT(digitalPinToPCINT(PIN_ENC_A), processDynamicScroll, CHANGE);
}

void processDynamicScroll()
{
    static unsigned long lastDetentTime = 0;

    byte rotationDirection = digitalRead(PIN_ENC_A) ? digitalRead(PIN_ENC_B) : (~digitalRead(PIN_ENC_B)) & 1;

    int factor = min(5, max(1, (20 - (signed long)(millis() - lastDetentTime)) / 4));

    factor = rotationDirection ? factor : -factor;

    applyCounterChange(factor);

    lastDetentTime = millis();
}

/**
 * Apply a change to the counter.
 */
void applyCounterChange(int factor)
{
    counter += factor;
    counter = (counter < 0) ? (360 + counter) : counter % 360;

    oled.setCursor(0, 3);
    oled.set2X();
    String counterString = String("000");
    counterString.concat(String(counter));
    counterString = counterString.substring(counterString.length() - 3);

    oled.print(counterString);
}

/**
 * Array of function pointers to setup the various modes.
 */
void (*modesSetters[2])() = {setupSimpleScroll, setupDynamicScroll};

/**
 * Print the mode header on the first line of the display.
 */
void printModeHeader(String modeHeader)
{
    oled.clear();
    oled.set1X();
    oled.print(modeHeader);
    applyCounterChange(0);
}

/**
 * Change to next mode. 
 */
void changeMode()
{
    mode = (mode + 1) % (sizeof(modesSetters) / sizeof(modesSetters[0]));
    initMode();
}

/**
 * Invode the setter function for the current mode.
 */
void initMode()
{
    modesSetters[mode]();
}

void setup()
{
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    oled.begin(&Adafruit128x64, DISPLAY_I2C_ADDRESS);
    oled.setFont(System5x7);

    attachPCINT(digitalPinToPCINT(PIN_ENC_SW), changeMode, RISING);

    initMode();
}

void loop()
{
}