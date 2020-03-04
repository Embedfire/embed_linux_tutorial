
#ifndef __OLED_APP_H
#define	__OLED_APP_H




#define I2C_RETRIES                                 0x0701
#define I2C_TIMEOUT                                 0x0702
#define I2C_SLAVE                                   0x0703       //IIC从器件的地址设置
#define I2C_BUS_MODE                                0x0780

#define OLED_COMMEND_ADDR 0x00
#define OLED_DATA_ADDR 0x40
#define OLED_ADDRESS 0x3C   //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78  3C  

typedef unsigned char uint8;



static uint8 oled_i2c_write(int fd, uint8 reg, uint8 val);
void OLED_Init(void);
void OLED_Fill(unsigned char fill_Data); //全屏填充
void OLED_CLS(void);//清屏
void oled_set_Pos(unsigned char x, unsigned char y); //设置起始点坐标
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);

#endif