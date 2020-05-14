# jhd1802/hd44780

Acknowledgement:
https://github.com/gorskima/hd44780-i2c/

<br>
This is a Linux kernel driver for JHD1802/HD44780 LCDs attached to I2C bus.
Ideal to use with PocketBeagle and other small devices running Linux where I2C bus is available.

`$ uname -snrmv | tr -d '\n' > /dev/lcd0`


### Features
The main goal was to expose HD44780-based LCDs behind regular Linux device files normally found in `/dev` directory. Thus, writing to the display is as easy as `echo Hello, world! > /dev/lcd0`.

There are no imposed limitations on number of concurrently attached devices. In practice, a single I2C bus allows up to 128 uniquely addressable devices. Furthermore, standard HD44780 LCD to I2C adapters usually use the same, hardcoded I2C address (like 0x3E) and it's not possible to change them.

Multiple LCD geometries are supported (20x4, 16x8[default] and 8x1) and it's trivial to add new ones if needed.

Supported escape sequences:
* `\r` - carriage return
* `\n` - line feed (new line)

Supported VT100 terminal control escape sequences:
* `<ESC>[H` - cursor home
* `<ESC>[2J` - erase screen

Device attributes exported via sysfs (`/sys/class/hd44780/<device_name>`):
* `backlight` - controls LCD backlight. Possible values: `0`, `1`
* `geometry` - sets LCD geometry. Possible values: `20x4`, `16x2`, `8x1`
* `cursor_blink` - controls cursor blink. Possible values: `0`, `1`
* `cursor_display` - displays or hides cursor. Possible values: `0`, `1`

### Usage
1. Insert kernel module: `modprobe hd44780`.
2. Let the I2C adapter know that there's a new device attached: `echo hd44780 0x27 > /sys/class/i2c-adapter/i2c-1/new_device`.
You may need to replace the device's I2C address and adapter path with proper values.
3. At this point a new device should appear (`/dev/lcd0`) and you should be able to write to it.
