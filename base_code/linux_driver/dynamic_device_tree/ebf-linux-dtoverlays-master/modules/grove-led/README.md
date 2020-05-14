GROVE GPIO Output device
========================

  After driver and configuration prepared well,
  the user could access sysfs path ```/sys/class/leds/<led>/*```,
  default to ```/sys/class/leds/grove:usr0/```

  The led name `<led>` specified in device tree source (.dts).

  For entries description of `<led>/*`, refer to
  [sysfs-class-led](https://github.com/raspberrypi/linux/blob/rpi-4.14.y/Documentation/ABI/testing/sysfs-class-led).

Usage
-----

***example:***

  ```bash
  # led on
  echo 1 > /sys/class/leds/grove:usr0/brightness
  # led off
  echo 0 > /sys/class/leds/grove:usr0/brightness
  ```

  If you need customize the GPIO-Port/LED-name/..., refer to device tree configuration
  [grove,led.txt](../../doc/devicetree/bindings/grove,led.txt)

