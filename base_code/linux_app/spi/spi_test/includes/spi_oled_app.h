#ifndef __SPI_OLED_APP_H
#define	__SPI_OLED_APP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



/* SPI3 总线设备文件路径*/
#define spi3_oled_DEV_path        "/dev/spidev2.0"




void spi_init(void);
void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len);

#endif