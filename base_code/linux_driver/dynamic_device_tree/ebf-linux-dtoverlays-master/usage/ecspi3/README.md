# ecspi测试笔记

## 编译
```bash
arm-linux-gnueabihf-gcc spidev_test.c -o spidev_test
```


## 运行
短接板子上的MOSI和MISO：针对pro板子的IO编号为IO1.22和IO1.23
```bash
./spidev_test -v -D /dev/spidev2.0 -p "123a"
```