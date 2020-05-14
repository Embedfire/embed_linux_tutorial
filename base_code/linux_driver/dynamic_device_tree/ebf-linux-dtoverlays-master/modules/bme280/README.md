GROVE Temp&Humi&Barometer Sensor (BME280)
=========================================

  After driver and configuration prepared well,
  the user could access ```/sys/bus/iio/devices/iio\:device<N>/XXX```,
  to get the sensor data results.

***

Usage
-----

***example:***

  ```bash
  # check the device<N> name, make sure it's hcsr04
  # where <N> is a number specific to your system.

  $ cat /sys/bus/iio/devices/iio\:device2/name
  bme280
    
  # get the temperature, in milli Celsius.
  $ cat /sys/bus/iio/devices/iio\:device2/in_temp_input
  25730

  # get the humidity, in 0.001%
  $ cat /sys/bus/iio/devices/iio\:device2/in_humidityrelative_input
  48560

  # get the air pressure, in kPa
  $ cat /sys/bus/iio/devices/iio\:device2/in_pressure_input
  101.390152343
  ```

  If you need customize the I2C-Port..., change it the device tree source.

