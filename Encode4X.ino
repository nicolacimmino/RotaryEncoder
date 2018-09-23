
// 4X encode mode.
//
// We count a step at every edge of A and B. Direction of travel is determined by
// the status of B at the rising edge of A and !B at the falling edge of A, conversely
// by the status of A at B falling edge and !A at be rising edge.
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

void setup4XEncode()
{
    detachPCINT(digitalPinToPCINT(PIN_ENC_A));
    detachPCINT(digitalPinToPCINT(PIN_ENC_B));
    attachPCINT(digitalPinToPCINT(PIN_ENC_A), process4XaEncode, CHANGE);
    attachPCINT(digitalPinToPCINT(PIN_ENC_B), process4XbEncode, CHANGE);
}

void process4XaEncode()
{
    uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(PIN_ENC_A));
    bool cwRotation = (trigger == RISING) ? digitalRead(PIN_ENC_B) : !digitalRead(PIN_ENC_B);
    applyCounterChange(cwRotation);
}

void process4XbEncode()
{
    uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(PIN_ENC_B));
    bool cwRotation = (trigger == FALLING) ? digitalRead(PIN_ENC_A) : !digitalRead(PIN_ENC_A);
    applyCounterChange(cwRotation);
}