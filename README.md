# SteeringWheelControl
An arduino library that reads steering wheel button events from a [Metra Axxess ASWC](http://amzn.com/B0039H2W66).

Based off of the IRremote library (Copyright 2009-2012 Ken Shirriff), you can find his github repository at: http://github.com/shirriff/Arduino-IRremote

The IRremote library has been trimmed down to only support the Alpine infrared codes that the Metra ASWC is capable of outputting.  The reduction in size and code complexity was desired for the particular project that I was working on at the time.  IRremote is great, but it's a swiss-army knife and can be pretty obtrusive if you're just wanting to handle a single use-case.  This has been tested to work, I used it in my mp3car circa around 2012, with no issues to speak of.

# Usage

* Purchase a [Metra Axxess ASWC](http://amzn.com/B0039H2W66).  This will run about $40-45 USD.
* The ASWC should be installed in your vehicle per the instructions.  In my car, the ASWC tapped in the CAN bus.
* The Program the AWSC to output Alpine radio codes.
* Wire up the ASWC to your Arduino.  I can't remember off-hand what the wiring order was, as it's been a while, but if I do find any notes, I'll list them here.  You're looking for ground an a ~5V TTL output.  I used a scope.  Be sure to ground your scope to the card ground and observe electrical safety rules when doing so.  Once you've got the output, wire it up to a digital pin. 
* Add the arduino pin # to your arduino sketch, it should be passed to the SwcRecv constructor.
* The code works with interrupts.  Make sure the rest of your code won't interfere with this, and that you're using a supported arduino.  In the main `loop()`, use `getStatus()` until the value is `SWC_DECODED`, then read the `.value` and then call `.resume()`.
