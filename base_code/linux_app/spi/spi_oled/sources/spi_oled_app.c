#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/spi/spidev.h>
#include "spi_oled_app.h"

static uint32_t mode = SPI_MODE_0;           //用于保存 SPI 工作模式
static uint8_t bits = 8;        // 接收、发送数据位数
static uint32_t speed = 500000; // 发送速度
static uint16_t delay;          //保存延时时间

/* 设备文件描述符 */
int data_or_command = 0; // 命令 、 数据 控制引脚的设备文件描述符
int fd;                  // SPI 控制引脚的设备文件描述符

int fd = -1;

/*程序中使用到的点阵 字库*/
extern const unsigned char F16x16[];
extern const unsigned char F6x8[][6];
extern const unsigned char F8X16[];

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
  /*------------------第一部分--------------------*/
  struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)tx,
      .rx_buf = (unsigned long)rx,
      .len = len,
      .delay_usecs = delay,
      .speed_hz = speed,
      .bits_per_word = bits,
      .tx_nbits = 1,
      .rx_nbits = 1
  };

  /*------------------第二部分--------------------*/
  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1)
  {
    pabort("can't send spi message");
  }
}



/*
* 向 SPI_OLED 发送控制命令
* cmd， 要发送的命令。 
*/
void spi_oled_send_commend(unsigned char cmd)
{
  uint8_t tx = cmd;
  uint8_t rx;

  write(data_or_command, "255", 3); //设置 SPI_OLED 的 D/C 为低电平
  transfer(fd, &tx, &rx, 1);        //发送控制命令
}

/*
* 向 SPI_OLED 发送数据
* cmd， 要发送的数据。 
*/
void spi_oled_send_data(unsigned char dat)
{
  uint8_t tx = dat;
  uint8_t rx;

  write(data_or_command, "0", 1); //设置 SPI_OLED 的 D/C 为高电平
  transfer(fd, &tx, &rx, 1);      //发送数据
}

/*
* 填充整个OLED 显示屏
* bmp_dat， 填充的值 
*/
void OLED_Fill(unsigned char bmp_dat) //全屏填充
{
  unsigned char y, x;
  for (y = 0; y < 8; y++)
  {
    spi_oled_send_commend(0xb0 + y);
    spi_oled_send_commend(0x01);
    spi_oled_send_commend(0x10);
    for (x = 0; x < X_WIDTH; x++)
    {
      spi_oled_send_data(bmp_dat);
    }
  }
}

/*
* 设置光标位置
* 参数： x, 目标位置的x轴坐标(取值范围 0到7 )。y， 目标位置的y轴坐标（取值范围 0到 X_WIDTH - 1）
*/
void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{
  spi_oled_send_commend(0xb0 + y);
  spi_oled_send_commend(((x & 0xf0) >> 4) | 0x10);
  spi_oled_send_commend((x & 0x0f) | 0x01);
}

/**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
	* @retval 无
  */
void OLED_ON(void)
{
  spi_oled_send_commend(0X8D); //设置电荷泵
  spi_oled_send_commend(0X14); //开启电荷泵
  spi_oled_send_commend(0XAF); //OLED唤醒
}

/**
  * @brief  OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
  * @param  无
	* @retval 无
  */
void OLED_OFF(void)
{
  spi_oled_send_commend(0X8D); //设置电荷泵
  spi_oled_send_commend(0X10); //关闭电荷泵
  spi_oled_send_commend(0XAE); //OLED休眠
}

/*
* oled 初始化函数
*/
void oled_init(void)
{
  spi_and_gpio_init();

  spi_oled_send_commend(0xae);
  spi_oled_send_commend(0xae); //--turn off oled panel
  spi_oled_send_commend(0x00); //---set low column address
  spi_oled_send_commend(0x10); //---set high column address
  spi_oled_send_commend(0x40); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  spi_oled_send_commend(0x81); //--set contrast control register
  spi_oled_send_commend(0xcf); // Set SEG Output Current Brightness
  spi_oled_send_commend(0xa1); //--Set SEG/Column Mapping     0xa0,0xa1
  spi_oled_send_commend(0xc8); //Set COM/Row Scan Direction   0xc0,0xc8
  spi_oled_send_commend(0xa6); //--set normal display
  spi_oled_send_commend(0xa8); //--set multiplex ratio(1 to 64)
  spi_oled_send_commend(0x3f); //--1/64 duty
  spi_oled_send_commend(0xd3); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
  spi_oled_send_commend(0x00); //-not offset
  spi_oled_send_commend(0xd5); //--set display clock divide ratio/oscillator frequency
  spi_oled_send_commend(0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
  spi_oled_send_commend(0xd9); //--set pre-charge period
  spi_oled_send_commend(0xf1); //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  spi_oled_send_commend(0xda); //--set com pins hardware configuration
  spi_oled_send_commend(0x12);
  spi_oled_send_commend(0xdb); //--set vcomh
  spi_oled_send_commend(0x40); //Set VCOM Deselect Level
  spi_oled_send_commend(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
  spi_oled_send_commend(0x02); //
  spi_oled_send_commend(0x8d); //--set Charge Pump enable/disable
  spi_oled_send_commend(0x14); //--set(0x10) disable
  spi_oled_send_commend(0xa4); // Disable Entire Display On (0xa4/0xa5)
  spi_oled_send_commend(0xa6); // Disable Inverse Display On (0xa6/a7)
  spi_oled_send_commend(0xaf); //--turn on oled panel

  OLED_Fill(0x00);
  OLED_SetPos(0, 0);
}



/*
* 初始化SPI ,初始化OLED 使用到的命令、数据控制引脚
*/
void spi_and_gpio_init(void)
{
  int ret = 0;

  /*打开 SPI 设备*/
  fd = open("/dev/spidev2.0", O_RDWR);
  if (fd < 0)
  {
    pabort("can't open /dev/spidev2.0 ");
  }

  /*打开 LED 设备*/
  data_or_command = open(data_or_command_DEV_path, O_RDWR);
  if (fd < 0)
  {
    pabort("can't open /sys/class/leds/green/brightness");
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





/**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
	*					ch[] :- 要显示的字符串; 
	*					TextSize : 字符大小(1:6*8 ; 2:8*16)
	* @retval 无
  */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
  unsigned char c = 0, i = 0, j = 0;
  switch (TextSize)
  {
  case 1:
  {
    while (ch[j] != '\0')
    {
      c = ch[j] - 32;
      if (x > 126)
      {
        x = 0;
        y++;
      }
      OLED_SetPos(x, y);
      for (i = 0; i < 6; i++)
        spi_oled_send_data(F6x8[c][i]);
      x += 6;
      j++;
    }
  }
  break;
  case 2:
  {
    while (ch[j] != '\0')
    {
      c = ch[j] - 32;
      if (x > 120)
      {
        x = 0;
        y++;
      }
      OLED_SetPos(x, y);
      for (i = 0; i < 8; i++)
        spi_oled_send_data(F8X16[c * 16 + i]);
      OLED_SetPos(x, y + 1);
      for (i = 0; i < 8; i++)
        spi_oled_send_data(F8X16[c * 16 + i + 8]);
      x += 8;
      j++;
    }
  }
  break;
  }
}


/**
  * @brief  OLED_ShowCN，显示codetab.h中的汉字,16*16点阵
  * @param  x,y: 起始点坐标(x:0~127, y:0~7); 
	*					N:汉字在codetab.h中的索引
	* @retval 无
  */
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
  unsigned char wm = 0;
  unsigned int adder = 32 * N;
  OLED_SetPos(x, y);
  for (wm = 0; wm < 16; wm++)
  {
    spi_oled_send_data(F16x16[adder]);
    adder += 1;
  }
  OLED_SetPos(x, y + 1);
  for (wm = 0; wm < 16; wm++)
  {
    spi_oled_send_data(F16x16[adder]);
    adder += 1;
  }
}



/**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
	*					x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
	* @retval 无
  */
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
  unsigned int j = 0;
  unsigned char x, y;

  if (y1 % 8 == 0)
    y = y1 / 8;
  else
    y = y1 / 8 + 1;
  for (y = y0; y < y1; y++)
  {
    OLED_SetPos(x0, y);
    for (x = x0; x < x1; x++)
    {
      spi_oled_send_data(BMP[j++]);
    }
  }
}
