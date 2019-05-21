//
// Rotary Encoder library for Arduino.
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

#include "RotaryEncoder.h"

void RotaryEncoder::begin(uint8_t pinA, uint8_t pinB, uint8_t pinSW,
                          uint8_t decodeMode = ROTARY_ENCODER_DECODE_MODE_2X,
                          uint8_t positionChangeMode = ROTARY_ENCODER_MODE_LINEAR,
                          int minPosition = 0, int maxPosition = 100)
{
    this->pinA = pinA;
    this->pinB = pinB;
    this->pinSW = pinSW;

    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(pinSW, INPUT_PULLUP);

    this->lastA = digitalRead(pinA);
    this->lastB = digitalRead(pinB);

    this->decodeMode = decodeMode;
    this->positionChangeMode = positionChangeMode;
    this->minPosition = minPosition;
    this->maxPosition = maxPosition;
    this->position = minPosition;
    this->onRotation = NULL;
    this->onClick = NULL;
    this->onLongPress = NULL;
}

void RotaryEncoder::registerOnRotationCallback(void (*callback)(bool cw, int position))
{
    this->onRotation = callback;
}

void RotaryEncoder::registerOnClickCallback(void (*callback)())
{
    this->onClick = callback;
}

void RotaryEncoder::registerOnLongPressCallback(void (*callback)())
{
    this->onLongPress = callback;
}

void RotaryEncoder::setDecodeMode(uint8_t mode)
{
    this->decodeMode = mode;
}

void RotaryEncoder::setPositionChangeMode(uint8_t mode)
{
    this->positionChangeMode = mode;
}

void RotaryEncoder::setMinPosition(int minPosition)
{
    this->minPosition = minPosition;
}

void RotaryEncoder::setMaxPosition(int maxPosition)
{
    this->maxPosition = maxPosition;
}

void RotaryEncoder::loop()
{
    this->encoderScanAB();
    this->encoderScanSwitch();
}

void RotaryEncoder::encoderScanAB()
{
    int currentA = digitalRead(this->pinA);
    int currentB = digitalRead(this->pinB);

    this->processABChange(this->getChange(this->lastA, currentA), this->getChange(this->lastB, currentB));

    this->lastA = currentA;
    this->lastB = currentB;
}

uint8_t RotaryEncoder::getChange(int oldValue, int newValue)
{
    if (oldValue == newValue)
    {
        return __ROTARY_ENCODER_NOCHANGE;
    }

    if (oldValue == LOW)
    {
        return __ROTARY_ENCODER_RISE;
    }

    return __ROTARY_ENCODER_FALL;
}

void RotaryEncoder::processABChange(uint8_t changeA, uint8_t changeB)
{
    if (changeA == __ROTARY_ENCODER_NOCHANGE && changeB == __ROTARY_ENCODER_NOCHANGE)
    {
        return;
    }

    switch (this->decodeMode)
    {
    case ROTARY_ENCODER_DECODE_MODE_1X:
        this->processABChange1X(changeA, changeB);
        break;
    case ROTARY_ENCODER_DECODE_MODE_2X:
        this->processABChange2X(changeA, changeB);
        break;
    case ROTARY_ENCODER_DECODE_MODE_4X:
        this->processABChange4X(changeA, changeB);
        break;

    default:
        break;
    }
}

void RotaryEncoder::processABChange1X(uint8_t changeA, uint8_t changeB)
{
    if (changeA == __ROTARY_ENCODER_RISE)
    {
        this->processRotationStep(digitalRead(this->pinB) == LOW);
    }
}

void RotaryEncoder::processABChange2X(uint8_t changeA, uint8_t changeB)
{
    if (changeA != __ROTARY_ENCODER_NOCHANGE)
    {
        this->processRotationStep(changeA == __ROTARY_ENCODER_RISE ? digitalRead(this->pinB) == LOW : digitalRead(this->pinB) == HIGH);
    }
}

void RotaryEncoder::processABChange4X(uint8_t changeA, uint8_t changeB)
{
    if (changeA != __ROTARY_ENCODER_NOCHANGE)
    {
        this->processRotationStep(changeA == __ROTARY_ENCODER_RISE ? digitalRead(this->pinB) == LOW : digitalRead(this->pinB) == HIGH);
    }

    if (changeB != __ROTARY_ENCODER_NOCHANGE)
    {
        this->processRotationStep(changeB == __ROTARY_ENCODER_RISE ? digitalRead(this->pinA) == HIGH : digitalRead(this->pinA) == LOW);
    }
}

void RotaryEncoder::processRotationStep(bool cw)
{
    Serial.print(cw);
    this->position += (this->getPositionIncrement() * (cw ? 1 : -1));
    unsigned int dynamicRange = 1 + this->maxPosition - this->minPosition;
    this->position = this->minPosition + (((dynamicRange + this->position) - this->minPosition) % (dynamicRange));

    if (this->onRotation != NULL)
    {
        this->onRotation(cw, this->position);
    }
}

unsigned int RotaryEncoder::getPositionIncrement()
{
    if (this->positionChangeMode == ROTARY_ENCODER_MODE_LINEAR)
    {
        return 1;
    }

    unsigned int factor = max(1, (20 - (signed long)(millis() - this->lastPositionChangeTime)) >> 2);
    this->lastPositionChangeTime = millis();

    return factor;
}

void RotaryEncoder::encoderScanSwitch()
{
    if (digitalRead(this->pinSW) != 0)
    {
        return;
    }

    // Allow the switch to stabilise.
    byte debounce = 0x55;
    while (debounce != 0x00)
    {
        debounce = (debounce < 1) | (digitalRead(this->pinSW) & 1);
        delay(1);
    }

    // Wait for the switch to be released or a timeout of 500mS to expire.
    unsigned long initialTime = millis();
    while ((millis() - initialTime < 500) && digitalRead(this->pinSW) == 0)
    {
        delay(1);
    }

    if (digitalRead(this->pinSW) == 0)
    {
        if (this->onLongPress != NULL)
        {
            this->onLongPress();
        }
    }
    else
    {
        if (this->onClick != NULL)
        {
            this->onClick();
        }
    }

    while (digitalRead(this->pinSW) == 0)
    {
        delay(1);
    }
}
