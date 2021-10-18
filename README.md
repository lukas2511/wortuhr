Word-Clock controller using Pi Pico (rp2040) Microcontroller

Time is given by an external battery-backed DS1307 RTC, driven via a
bi-directional voltage shifter (DS1307 is 5V only, Pi Pico is 3.3V only).
Doing it this way as I'm just using what I had on hand.

A small I2C OLED display on the backside is being used to display date
and time, with a single button for configuration.

In this setup each word(-segment) is connected directly to the pico.
There are 24 segments which need to be controlled, so I need 24 GPIO pins.

In theory the pico has 26 GPIO pins, but two are needed for I2C and
I needed another one for the button. Luckily the LED can easily be
desoldered and with a wire added I got another GPIO to use.

I'm trying to keep the time on DS1307 in UTC, so I have a fixed time-base
to calculate if I need to add a +1h offset for DST.

The clock can either be set using the button and display, or quickly via
the usb serial console of the pico:
`date -u +"S%Y-%m-%d %H:%M:%SF" > /dev/ttyACM0`

!["Screenshot"](https://raw.githubusercontent.com/lukas2511/wortuhr/potato/screenshot-front.jpg)

!["Screenshot2"](https://raw.githubusercontent.com/lukas2511/wortuhr/potato/screenshot-back.jpg)


