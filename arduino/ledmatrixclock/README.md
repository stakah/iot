# LED Matrix clock

![Picture of the Project][capture-img]

## BOM

* Arduino Uno
* [8x32 LED Matrix module with MAX7219 board][amz-led]

## Pin Connections

|Arduino Pin| LED Board Pin|
|-----------|--------------|
| +5V       |    VCC       |
| GND       |    GND       |
| Pin 10    |    CLK       |
| Pin  9    |    CS        |
| Pin  8    |    DIN       |

## Releases

### v0.1

  First iteration with simple delay in loop function to measure time.

  Simple test shown about 3 hours delay per 24 hours cycle. 

### v0.2

  Second iteration. Instead of using delay in loop function, use Hardware timer interrupt to count time
  and increase conters. Espected to be more precise than v0.1.
----
[capture-img]: images/20210627_135538.jpg "Capture"
[amz-led]: https://www.amazon.com/MAX7219-Control-Display-Max7219-Controller/dp/B07GWV2GD7