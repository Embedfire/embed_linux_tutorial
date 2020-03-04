
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#include "oled_app.h"



int fd = -1;

/*程序中使用到的点阵 字库*/
extern const unsigned char F16x16[];
extern const unsigned char F6x8[][6];
extern const unsigned char F8X16[];
extern const unsigned char BMP1[];


/* IIC 写函数
*   参数说明：fd，打开的设备文件句柄。 rg, 命令值。 val，要写入的数据
*   返回值：  成功，返回0. 失败，返回 -1
*/
static uint8 oled_i2c_write(int fd, uint8 reg, uint8 val)
{
    int retries;
    uint8 data[2];
    int write_error = 0;

    data[0] = reg;
    data[1] = val;
    
    ioctl(fd, I2C_SLAVE, OLED_ADDRESS);

    for (retries = 5; retries; retries--)
    {
        if (write(fd, data, 2) == 2)
        {
            return 0;
        }
        usleep(1000);
    }
    return -1;
}


// /* IIC 写函数
// *   参数说明：fd，打开的设备文件句柄。 rg, 命令值。 val，要写入的数据
// *   返回值：  成功，返回0. 失败，返回 -1
// */
// static uint8 oled_oled_i2c_write_command(int fd, uint8 reg, uint8 val)
// {
//     int retries;
//     uint8 data[2];
//     int write_error = 0;

//     data[0] = reg;
//     data[1] = val;
    
//     ioctl(fd, I2C_SLAVE, OLED_ADDRESS);

//     for (retries = 5; retries; retries--)
//     {
//         if (write(fd, data, 2) == 2)
//         {
//             return 0;
//         }
//         usleep(1000);
//     }
//     return -1;
// }









/*  初始化 OLED 
*   参数说明：fd，打开的设备文件句柄。 rg, 命令值。 val，要写入的数据
*   返回值：  成功，返回0. 失败，返回 -1
*/

void OLED_Init(void)
{
    fd = open("/dev/i2c-0", O_RDWR); // open file and enable read and  write

    if (fd < 0)
    {
        perror("Can't open /dev/i2c-0 \n"); // open i2c dev file fail
        exit(1);
    }
    
    /*发送 从设备地址， 这里就是 oled的地址*/
    if (ioctl(fd, I2C_SLAVE, OLED_ADDRESS) < 0)
    { //set i2c address
        printf("fail to set i2c device slave address!\n");
        close(fd);
    }


    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xAE); //display off
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x20); //Set Memory Addressing Mode
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xb0); //Set Page Start Address for Page Addressing Mode,0-7
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xc8); //Set COM Output Scan Direction
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x00); //---set low column address
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x10); //---set high column address
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x40); //--set start line address
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x81); //--set contrast control register
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xf);  //亮度调节 0x00~0xff
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xa1); //--set segment re-map 0 to 127
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xa6); //--set normal display
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xa8); //--set multiplex ratio(1 to 64)
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x3F); //
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xd3); //-set display offset
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x00); //-not offset
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xd5); //--set display clock divide ratio/oscillator frequency
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xf0); //--set divide ratio
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xd9); //--set pre-charge period
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x22); //
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xda); //--set com pins hardware configuration
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x12);
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xdb); //--set vcomh
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x20); //0x20,0.77xVcc
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x8d); //--set DC-DC enable
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x14); //
    oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xaf); //--turn on oled panel
}



/**
  * @brief  OLED_Fill，填充整个屏幕
  * @param  fill_Data:要填充的数据
	* @retval 无
  */
void OLED_Fill(unsigned char fill_Data) //全屏填充
{
    unsigned char m, n;
    for (m = 0; m < 8; m++)
    {
        oled_i2c_write(fd, OLED_COMMEND_ADDR, 0xb0 + m); //page0-page1
        oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x00);     //low column start address
        oled_i2c_write(fd, OLED_COMMEND_ADDR, 0x10);     //high column start address

        for (n = 0; n < 128; n++)
        {
            // WriteDat(fill_Data);
            oled_i2c_write(fd, OLED_DATA_ADDR, fill_Data); //high column start address
        }
    }
}


 /**
  * @brief  OLED_SetPos，设置光标
  * @param  x,光标x位置  y，光标y位置
  *					
  * @retval 无
  */
void oled_set_Pos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	oled_i2c_write(fd, OLED_COMMEND_ADDR,0xb0+y);
	oled_i2c_write(fd, OLED_COMMEND_ADDR,((x&0xf0)>>4)|0x10);
	oled_i2c_write(fd, OLED_COMMEND_ADDR,(x&0x0f)|0x01);
}



 /**
  * @brief  OLED_CLS，清屏
  * @param  无
	* @retval 无
  */
void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}


 /**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
	* @retval 无
  */
void OLED_ON(void)
{
	oled_i2c_write(fd, OLED_COMMEND_ADDR,0X8D);  //设置电荷泵
	oled_i2c_write(fd, OLED_COMMEND_ADDR,0X14);  //开启电荷泵
	oled_i2c_write(fd, OLED_COMMEND_ADDR,0XAF);  //OLED唤醒
}


 /**
  * @brief  OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
  * @param  无
	* @retval 无
  */
void OLED_OFF(void)
{
	oled_i2c_write(fd, OLED_COMMEND_ADDR,0X8D);  //设置电荷泵
	oled_i2c_write(fd, OLED_COMMEND_ADDR,0X10);  //关闭电荷泵
	oled_i2c_write(fd, OLED_COMMEND_ADDR,0XAE);  //OLED休眠
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
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				oled_set_Pos(x,y);
				for(i=0;i<6;i++)
					oled_i2c_write(fd, OLED_DATA_ADDR,F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}  
				oled_set_Pos(x,y);
				for(i=0;i<8;i++)
					oled_i2c_write(fd, OLED_DATA_ADDR,F8X16[c*16+i]);
				oled_set_Pos(x,y+1);
				for(i=0;i<8;i++)
					oled_i2c_write(fd, OLED_DATA_ADDR,F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
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
	unsigned char wm=0;
	unsigned int  adder=32*N;
	oled_set_Pos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		oled_i2c_write(fd, OLED_DATA_ADDR,F16x16[adder]);
		adder += 1;
	}
	oled_set_Pos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
	  oled_i2c_write(fd, OLED_DATA_ADDR,F16x16[adder]);
		adder += 1;
	}
}




 /**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
	*					x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
	* @retval 无
  */
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		oled_set_Pos(x0,y);
    for(x=x0;x<x1;x++)
		{
			oled_i2c_write(fd, OLED_DATA_ADDR,BMP[j++]);
		}
	}
}

