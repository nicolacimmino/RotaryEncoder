# Rotary Encoder #

This is a reference implementation for reading a rotary encoder. A rotary encoder is a device that allows to measure the angular rotation of its shaft as well as the direction of rotation. It outputs two signals, to which we will refer throughout the text as A and B. These two signals are shifted by 90 degrees which is why rotary encoders are also called "quadrature encoders". There are rotary encoders meant for human operation, for instance a control knob on a device front panel, and others that are meant to detect speed and direction of rotation of motors or any turning shaft for that matter. The ones meant for human opeation have quite marked detents to give tactile feedback.

## Angular Distance ##

As you can see in the timing diagram below, if we are interested only in knowing the angular distance travelled by the shaft, we can just count the pulses on one of the two channels. Rotary encoders specs always include a number of Pulses Per Revolution (PPR), so we can know how many degrees the shaft has travelled. For instance an encoder with 20 PPR will have travelled 180 degrees after 10 rising edges of A.

![Travelled Distance](documentation/travelled_distance.png)

In other words a rotary encoder with 20 PPR has a resolution of 18 degrees per pulse. We could though double this resolution by detecting both the raising and falling edges of A. As seen below we get the counter incrementing on both edges which gives us a resolution of 6 degrees. This mode is usually referred to a 2X. Note that this mode (as well as 4X introduced below) are only different ways to decode the signal, they are not a characteristic of the encoder per se. Any quadrature encoder can be read in 1x, 2x or 4x mode.

![Travelled Distance 2X](documentation/travelled_distance-2x.png)

If we add the B signal into the equation and count on both rising and falling edges of A and B we can increase the resolution by a factor of 4. So our 20 PPR encoder can be read with a resolution of 3 degrees. The timing diagram below shows the 4x decoding mode.

![Travelled Distance 4X](documentation/travelled_distance-4x.png)

In general the travelled angular distance can be expressed as:

![Distance Formula](documentation/distance_equation.gif)

Where P is the counted pulses (edges of the signal), PPR is the Pulses Per Revolution and Nx is the multiplier factor given by the decoding mode (1, 2, 4).

## Why don't we just use 4x? ##

Since the 4x mode gives higher resolution it might appear this is the only mode in which an encoder should be read. There are reasons though to use the other modes as well. In some cases, more rare though nowadays where microcontrollers are in pretty much every project, simplicity of hardware is one factor. You can simply hook up the A signal to a the clock of a counter, in which case you would be reading in 1x. For applications geared towards human interaction another important factor is the amount of detents versus PPR. For instance an encoder could have 30 detents and 15 PPR. If read in 1x mode it could result rather annoying as it requires to travel through two detents for an action to be taken (think, for instance, moving to the next entry on a menu), 2x would be the appropriate choice in this case. 4x would be completely out of the question as there would be an action between detents meaning you would be to set a specific item in the manu enties list.

Additionally increasing resolution does not equate to increasing precision, by reading an encoder in 4x mode also the errors are mutiplied by the same factor so, in practice, the accuracy is the same.

## Direction of travel ##

The second interesting characteristic of quadrature encoders is that they allow to detect in which direction the shaft is rotating. To understand how this is achieved see the timing diagram below which, we will assume, is representative of the signals when the shaft is travelling clockwise.

![1x-CW](documentation/1x-CW.png)

Notice how B is always *low* on the rising edge of A. If you now follow the above diagram in the opposite direction, reading it from right to left, which is what would happen if you turned the shaft counter-clockwise, you will see that B will always be *high* on the rising edges of A, this is becasue, effectively, the falling edges of A have become the rising ones. The timing diagram below shows the signals when the encoder is turning counter-clockwise.

![1x-CCW](documentation/1x-CCW.png)

So, simply by reading the value of B on the rising edge of A we can determine the direction of rotation. You can see an example of this in `Encode1x.ino` where an interrupt is attacched to the A signal rising edge. When the interrupt triggers the ISR we count one event determing the direction of travel by reading B.

When moving to 2x decoding we will need to determine the direction of travel by looking at B, as above, for the rising edge and the B negated on the falling edge, as should become apparent from the timing diagram below. You can see an example of this in `Encode2x.ino`.

![2x-CW](documentation/2x-CW.png)

Finally, moving to 4x encoding is just one more step forward from the above progression. We will need to determine the rotation based on B values on A edges and on A values on B edges. You can see an example of this in `Encode4x.ino`.

![4x-CW](documentation/4x-CW.png)

## Application in human interfaces ##

Depending on the application, expecially when dealing with human interfaces, further considerations might apply. For instance assume you are controlling a menu selection where items are selected by turning the knob on the encoder. In this case it's mandatory that at each detent the selection moves by one entry. Anything else would feel awkward or plainly unusable (think a move of 2 items per detent!). This limits the selection of the encoder to one where the detents to PPR ratio is either 1, 2 or 4. You will then need to read in the appropriate mode to achieve one action per detent.

Consider instead an application where the knob is used to adjust a value with a relatively large dynamic range, for instance 0 to 360 as in the example code. Making large changes with an increment of 1 per detent would be incredibly annoying as it would take a long time. On the other hand, if the requirement is to be able to asjust the value with a unit resolution, increasing the increment per detent wouldn't be an option. It's at this point that we need to consider another dimension, which is the speed of rotation. We generally assume that when making fast adjustments, a human  wouldn't make precise ones and, conversely, when making precise adjustments a human would move the control more slowly. On this premise we can then move in larger steps when the control is being handled at higer speeds. Since, usually, it's easier to measure time intevals we can just invert the relation and save our code from performing time consuming (in the realm of small microcontrollers), we can then say that we need to use smaller increments as the time interval between detents increases.

![Dynamic Step Size](documentation/dynamic.png)

The graph above refers to the curve I have used in my example code, see `RotatyEncoder.ino`. By no means this is a hard curve suitable for all applications. The number of detents per revolution of your encoder, the dynamic range but also phyical elements such as the diameter of the knob will affect what feels better in practical usage. The curve I used is `y=(20-x)/4` clipped at 1 on the lower bound. Also, in the interest of avoiding divisions, the division by 4 is done by shifiting right by 2: `max(1, (20 - (signed long)(millis() - lastDetentTime)) >> 2);`
        

## Click Button ##

While not strictly realted to rotaty encoders a couple of notes regarding the click button are in place. Rotary encoders meant for human interfaces can have a push button integrated, this combination can be used to create single control intefaces which allow for navigation of menus and set of values. In the example code I made use of both a short and a long press. The first will rotate through the 1x,2x and 4x modes while the second will change between the linear and dynamic increment modes. All logic can be found in `changeMode()` in the example code. 

The first item of interest is the debouncing of the switch. Since this is a push button there will be chatter and, since the following code depends on measuring for how long the button is pressed, we need to make sure that has stopped. The code below does that by implementing a buffer, initialised to 0x55 (binary B01010101). Subsequent reads of the switch are pushed into the lsb rotating the all buffer left.

![Debounce](documentation/debounce.png)

The loop continues until the buffer contains all zeros, indicating 8 consecutive zero readings have been taken. There's nothing too magic about the value 0x55, in this case 0x01 would do just as well (0x80 clearly not as it would be enough for the first read to be 0 to exit the loop). Being 0x55 and alternation of 0s and 1s it's a good start value whether you are debouncing a switch that is holding an input to zero or one.

The next bit is a loop that will either timeout, after 500mS or exit if the button is releasead. This allows to react immediately to button release, which is expected, but also gives an immediate feedback to a user performing a long press when he kept it press long enough. 

![Debounce2](documentation/debounce2.png)

A more naive implementation that just waits for the button to be released and then times the duration of the press would leave the user wondering whether he pressed the button long enough.

From here the rest should be pretty much self explanatory, action is taken based on the type of press and the function waits for the button to be released before exiting to avoid unwanted re-entry. A final note: the code handling the button is not triggered by an interrupt because it relies on measuring times with `millis()` which is not updating during interrupt servicing (as timer interrupts are waiting for your ISR to end!). Additionally since the function can execute for a very long time, in fact as long as the user keeps the button depressed, it can cause many other side effects if handled in an ISR (eg loss of serial port data). While this is very Arduino specific, it's generally good practice not to have long executing tasks in ISRs.

## The Hardware ##

I build a simple prototype to test the code and get a first-hand impression of how the different solutions feel. If you waant to build one see the `#define` at the top of `RotaryEncoder.ino` for wiring. The display is a monochrome (fixed color is more apt, as the first line is yellow the rest it blue) SSD1306 with I2C interface. The code should be easily adaptable to any other display. The encoder is a 30 detent, 15 PPR encoder with a push button. If your encoder specs are different you might need to adjust the code, in particular the function the calculates the increment in the dymamic increment mode as it might feel too "slippery" or still too slow.


![Prototype](documentation/prototype.png)
