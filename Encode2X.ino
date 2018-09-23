
// X2 encode mode.
// 
// We count a step at every edge of A. Direction of travel is determined by
// the status of B at the rising edge of A and !B at the falling edge of A.
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

void setup2XEncode()
{
    detachPCINT(digitalPinToPCINT(PIN_ENC_A));
    detachPCINT(digitalPinToPCINT(PIN_ENC_B));
    attachPCINT(digitalPinToPCINT(PIN_ENC_A), process2XEncode, CHANGE);
}

void process2XEncode()
{
    uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(PIN_ENC_A));
    bool cwRotation = (trigger == RISING) ? digitalRead(PIN_ENC_B) : !digitalRead(PIN_ENC_B);
    applyCounterChange(cwRotation);
}