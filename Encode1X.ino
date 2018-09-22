
/**
 * X1 encode mode.
 * 
 * We register an interrupt on the rising edge of A and simply sample B and determine direction of 
 * roation based on the value of B.
 * 
 * - Prone to skipping on bad encoders with a lot of chatter.
 * - Response only every second detent as we process a single edge of A.
 * - Tedius for large changes.
 */

void setup1XEncode()
{
    detachPCINT(digitalPinToPCINT(PIN_ENC_A));
    detachPCINT(digitalPinToPCINT(PIN_ENC_B));
    attachPCINT(digitalPinToPCINT(PIN_ENC_A), process1XEncode, RISING);
}

void process1XEncode()
{    
    bool cwRotation = digitalRead(PIN_ENC_B);
    applyCounterChange(cwRotation);
}