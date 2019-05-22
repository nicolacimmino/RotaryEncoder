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

#ifndef __ROTARY_ENCODER_H__
#define __ROTARY_ENCODER_H__

#define ROTARY_ENCODER_DECODE_MODE_1X 1
#define ROTARY_ENCODER_DECODE_MODE_2X 2
#define ROTARY_ENCODER_DECODE_MODE_4X 4

#define ROTARY_ENCODER_MODE_LINEAR 0
#define ROTARY_ENCODER_MODE_DYNAMIC 1

/*
 * Constants used only internally.
 */
#define __ROTARY_ENCODER_NOCHANGE 0
#define __ROTARY_ENCODER_RISE 1
#define __ROTARY_ENCODER_FALL 2

#include <Arduino.h>

class RotaryEncoder
{
public:
    /*
     * Constructor.
     * Only pins need to be set at this stage, other parameters can be modified also through setters.
     */
    void begin(uint8_t pinA, uint8_t pinB, uint8_t pinSW,
                          uint8_t decodeMode = ROTARY_ENCODER_DECODE_MODE_2X,
                          uint8_t positionChangeMode = ROTARY_ENCODER_MODE_LINEAR,
                          int minPosition = 0, int maxPosition = 100);

    /*
     * Loop needs to be called continuosly from the Arduino loop().
     * Eventual callbacks for click, long press and rotation are processed syncronously before this call is returned.
     */
    void loop();

    /*
     * Register function to be called on rotation. 
     * The function will receive the direction and the current position of the encoder.
     */
    void registerOnRotationCallback(void (*callback)(bool cw, int position));

    /*
     * Register function to be called on click. 
     */
    void registerOnClickCallback(void (*callback)());

    /*
     * Register function to be called on a long click. 
     */
    void registerOnLongPressCallback(void (*callback)());

    /*
     * Setter for the decode mode (1x, 2x, 4x).
     */
    void setDecodeMode(uint8_t mode);

    /*
     * Setter for the position change mode (linear, dynamic).
     */
    void setPositionChangeMode(uint8_t mode);

    /*
     * Setter for the minimum position.
     * If you keep rotating CCW after this value the position will wrap around at maxPosition.
     */
    void setMinPosition(int minPosition);

    /*
     * Setter for the maximum position.
     * If you keep rotating CW after this value the position will wrap around at minPosition.
     */
    void setMaxPosition(int maxPosition);

private:
    void (*onRotation)(bool cw, int position);
    void (*onClick)();
    void (*onLongPress)();
    uint8_t pinA;
    uint8_t pinB;
    uint8_t pinSW;
    uint8_t decodeMode;
    uint8_t positionChangeMode;
    int minPosition;
    int maxPosition;
    int position;
    unsigned long lastPositionChangeTime;
    int lastA = HIGH;
    int lastB = HIGH;
    void encoderScanSwitch();
    void encoderScanAB();
    uint8_t getChange(int oldValue, int newValue);
    void processRotationStep(bool cw);
    unsigned int getPositionIncrement();
    void processABChange(uint8_t changeA, uint8_t changeB);
    void processABChange1X(uint8_t changeA, uint8_t changeB);
    void processABChange2X(uint8_t changeA, uint8_t changeB);
    void processABChange4X(uint8_t changeA, uint8_t changeB);
};

#endif
