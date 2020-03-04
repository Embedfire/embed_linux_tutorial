#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/spi/spidev.h>
#include "spi_oled_app.h"

static  uint32_t mode;           //用于保存 SPI 工作模式
static  uint8_t bits = 8;        // 接收、发送数据位数
static  uint32_t speed = 500000; // 发送速度
static  uint16_t delay;          //保存延时时间

/* 设备文件描述符 */
int fd;                  // SPI 控制引脚的设备文件描述符



/*
* 错误处理函数。
*/
void pabort(const char *s)
{
  perror(s);
  abort();
}

/*
* 执行SPI 发送函数
* fd， SPI设备描述符。 tx, 要发送的数据，rx，接收到的数据首地址， 发送数据长度(单位，字节)
*/
void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
  int ret;
  int out_fd;
  struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)tx,
      .rx_buf = (unsigned long)rx,
      .len = len,
      .delay_usecs = delay,
      .speed_hz = speed,
      .bits_per_word = bits,
  };



  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1)
  {
    pabort("can't send spi message");
  }
}



/*
* 初始化SPI ,初始化OLED 使用到的命令、数据控制引脚
*/
void spi_init(void)
{
  int ret = 0;

  /*打开 SPI 设备*/
  fd = open("/dev/spidev2.0", O_RDWR);
  if (fd < 0)
  {
    pabort("can't open /dev/spidev2.0 ");
  }

  /*
	 * spi mode 设置SPI 工作模式
	 */
  ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
  if (ret == -1)
    pabort("can't set spi mode");

  ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
  if (ret == -1)
    pabort("can't get spi mode");

  /*
	 * bits per word  设置一个字节的位数
	 */
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't set bits per word");

  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't get bits per word");

  /*
	 * max speed hz  设置SPI 最高工作频率
	 */
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't set max speed hz");

  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't get max speed hz");

  printf("spi mode: 0x%x\n", mode);
  printf("bits per word: %d\n", bits);
  printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);
}










