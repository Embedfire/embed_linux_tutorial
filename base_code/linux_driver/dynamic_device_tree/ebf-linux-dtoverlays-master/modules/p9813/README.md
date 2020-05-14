GROVE Chainable RGB LED (P9813)
===============================

  After driver and configuration prepared well,
  the user could access ```/dev/p981x<N>```,
  to operate the chained RGB LEDs.

***

Usage
-----

***commands***

```
N chain-length
D led-index red green blue
  led-index be in [0, chain-length)
  red/green/blue be in [0, 255(0xFF)]
```

***example:***

  ```bash
  # check the device name
  ls -l /dev/p981x*
  # where <N> is a number start from 0,1...

  # specify the chain length (how many LEDs chained)
  # here is 2 leds chained.
  $ echo "N 2" > /dev/p981x0
    
  # set LED 0 to RED
  $ echo "D 0 0xFF 0 0" > /dev/p981x0

  # set LED 1 to GREEN
  $ echo "D 1 0 0xFF 0" > /dev/p981x0
  ```

  If you need customize the GPIO-Port..., refer to device tree document
  [p9813.txt](../../doc/devicetree/bindings/p9813.txt)

