## Grove Temperature sensor SHT31

### Introduction of sensor
SHT3x-DIS is the next generation of Sensirion’s temperature and humidity sensors. It builds on a new CMOSens® sensor chip that is at the heart of Sensirion’s new humidity and temperature platform.   



## Usage: 
* Get sht31's data by instruction below:

        sudo cat /dev/sht3x_dev<0~n>
        The number <0~n> depends on how many sht3x devices attached,default is 0.

* The console will print msg below:

        temperature = 23.68℃
        humidity = 69.15%
        temperature = 23.65℃
        humidity = 69.5%
        ..
  User can test the sensor with warming it.

***


