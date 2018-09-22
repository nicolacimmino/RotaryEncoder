
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

extern void setup1XEncode();
extern void setup2XEncode();
extern void setup4XEncode();

/**
 * Array of function pointers to setup the various modes.
 */
void (*modesSetters[3])() = {setup1XEncode, setup2XEncode, setup4XEncode};

String modeHeaders[3] = {"1X    ", "  2X  ", "    4X"};

String dynamicModeHeaders[2] = {"  LIN", "     EXP"};

/**
 * Change mode. 
 */
void changeMode()
{
    byte debounce = 0x55;
    while (debounce != 0x00)
    {
        debounce = (debounce < 1) | (digitalRead(PIN_ENC_SW) & 1);
        delay(1);
    }

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
}

/**
 * Apply a change to the counter.
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

void printHeader()
{
    oled.setCursor(0, 0);
    oled.set1X();
    oled.print(modeHeaders[encoderMode]);
    oled.print(dynamicModeHeaders[encoderDynamicMode]);
    oled.clearToEOL();
}

void loop()
{
    if (digitalRead(PIN_ENC_SW) == 0)
    {
        changeMode();

        printHeader();

        while (digitalRead(PIN_ENC_SW) == 0)
        {
            delay(1);
        }
    }

    oled.setCursor(0, 3);
    oled.set2X();
    String counterString = String("000");
    counterString.concat(String(counter));
    counterString = counterString.substring(counterString.length() - 3);
    oled.print(counterString);
}