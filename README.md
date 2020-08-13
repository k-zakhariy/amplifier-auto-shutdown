# Auto power off Stereo Amplifier if idle for too long

I wrote this program as I tend to forget the amplifier on and
- given the tube has limited lifetime, I am always feeling bad when I
- see it still on few hours later. Not to mention of course that we 
- can be a bit more green by preserving power.

The program is written for Arduino, basically reading left and right
signals on Analog inputs (A0 and A1) from a line level

If no music is detected for some time (constant idleTimeToTurnOff)
then the Arduino will send a power-off to a relay

Sampling the audio in every 2 seconds is reasonable and idle time is
set to 5 minutes. 

The audio sampling is not done in one shot, as we can randomly find
the Arduino ADC measuring 0, so we sample 10 consequential on each
channel (left and right). The detection uses simple max on all samples.

I found it mandatory to use a load resistor on the A0 and A1 since when
the Nova amplifier is off, the ADC on the Arduino is reading very random
numbers. Using the load resistor to ground `(10K Ohm)` made the samples
on amplifier off to 0.

The Arduino have enough IO ports still, so I am taking some of the internal
state and expose it with LEDs, please see below. These LEDs are optional
and connect/assembly only if you like. These are the LEDs that I used:
```
 Pin   Meaning   LED
 ------- ----------- -----------------
 13    Power     (on-board)
 12    Playing   Green
 11    Idle      Yellow
 10    Amplifier Blue
```

- The Blue LED will light up if the unit will detect the amplifier is on.
- The Yellow LED will light up if the unit will not detect any music.
- The Green LED will light up if the unit will detect music is playing.
- The Power LED will be on by default and blink off on each sample.