GROVE 3-Axis Digital Accelerometer(+-16g)
=========================================

  After driver and configuration prepared well,
  the user could access ```/dev/input/event<N>```,
  to capture the accelerometer event.

  For more description of ```/dev/input/event<N>```, refer to
  [input](https://github.com/raspberrypi/linux/blob/rpi-4.14.y/Documentation/input/input.rst).

Usage
-----

***example:***

  ```bash
  sudo apt-get install evtest

  evtest
  # now select the number <N> of event<N>
  # then move the sensor,
  #
  # see the evtest output, unit in 1/256 g.

  Input driver version is 1.0.1
  Input device ID: bus 0x18 vendor 0x0 product 0x159 version 0x0
  Input device name: "ADXL34x accelerometer"
  Supported events:
  Event type 0 (EV_SYN)
  Event type 1 (EV_KEY)
    Event code 330 (BTN_TOUCH)
  Event type 3 (EV_ABS)
    Event code 0 (ABS_X)
      Value      2
      Min    -4096
      Max     4096
      Fuzz       3
      Flat       3
    Event code 1 (ABS_Y)
      Value    -28
      Min    -4096
      Max     4096
      Fuzz       3
      Flat       3
    Event code 2 (ABS_Z)
      Value    254
      Min    -4096
      Max     4096
      Fuzz       3
      Flat       3
  Properties:
  Testing ... (interrupt to exit)
  Event: time 1552637001.573312, type 3 (EV_ABS), code 0 (ABS_X), value -81
  Event: time 1552637001.573312, type 3 (EV_ABS), code 1 (ABS_Y), value -114
  Event: time 1552637001.573312, type 3 (EV_ABS), code 2 (ABS_Z), value 203
  Event: time 1552637001.573312, -------------- SYN_REPORT ------------
  Event: time 1552637001.621290, type 3 (EV_ABS), code 0 (ABS_X), value -82
  Event: time 1552637001.621290, type 3 (EV_ABS), code 1 (ABS_Y), value -116
  Event: time 1552637001.621290, type 3 (EV_ABS), code 2 (ABS_Z), value 214
  ```

  If you need customize the I2C-Port..., change it the device tree source.

