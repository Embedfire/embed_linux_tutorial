GROVE GPIO Input device
=======================

  After driver and configuration prepared well,
  the user could access ```/dev/input/event<N>```,
  to capture the GPIO input event.

  For more description of ```/dev/input/event<N>```, refer to
  [input](https://github.com/raspberrypi/linux/blob/rpi-4.14.y/Documentation/input/input.rst).

Usage
-----

***example:***

  ```bash
  sudo apt-get install evtest

  evtest
  # now select the number <N> of event<N>
  # then pressing the button(change Grove device output level),
  # see the evtest output
  ```

  If you need customize the GPIO-Port/key-code/label..., refer to device tree configuration
  [grove,button.txt](../../doc/devicetree/bindings/grove,button.txt)

