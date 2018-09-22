
/**
 * X2 encode mode.
 * 
 * We register an interrupt on the rising edge of A and simply sample B and determine direction of 
 * roation based on the value of B.
 * 
 * - Prone to skipping on bad encoders with a lot of chatter.
 * - Response only every second detent as we process a single edge of A.
 * - Tedius for large changes.
 */

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