GROVE Ultrasonic Ranger (HCSR04)
================================

  After driver and configuration prepared well,
  the user could access ```/sys/bus/iio/devices/iio\:device<N>/in_distance_input```,
  to get distance between the sensor and object.

***

Usage
-----

***example:***

  ```bash
  # check the device<N> name, make sure it's hcsr04
  # where <N> is a number specific to your system.

  $ cat /sys/bus/iio/devices/iio\:device1/name
  hcsr04_1057@20
    
  # get the distance, in unit cm.
  cat /sys/bus/iio/devices/iio\:device1/in_distance_input
  2128
  ```

  If you need customize the GPIO-Port..., refer to device tree document
  [hcsr04.txt](../../doc/devicetree/bindings/hcsr04.txt)

