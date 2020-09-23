/*********************************************************************************
  *Copyright(C),    embedfire
  *FileName:        ap3216c
  *Author:          lixiaojian
  *Version:         v1.0
  *Date:            20200709
  *Description:     ap3216c
  *Others:          //其他内容说明
*********************************************************************************/

#ifndef AP3216C_H
#define AP3216C_H
#define AP3216C_ADDR    	        (0x1e)	/* ap3216c器件地址  */
 
#define   AP3216C_SYSTEMCONG	    (0x00)	/* 配置寄存器       */
#define   AP3216C_INTSTATUS	      (0x01)	/* 中断状态寄存器   */
#define   AP3216C_INTCLEAR	      (0x02)	/* 中断清除寄存器   */
#define   AP3216C_IRDATALOW	      (0x0a)	/* IR数据低8bit     */
#define   AP3216C_IRDATAHIGH	    (0x0b)	/* IR数据高8bit     */
#define   AP3216C_ALSDATALOW	    (0x0c)	/* ALS数据低8bit    */
#define   AP3216C_ALSDATAHIGH	    (0x0d)	/* ALS数据高8bit    */
#define   AP3216C_PSDATALOW	      (0x0e)	/* PS数据低8bit     */
#define   AP3216C_PSDATAHIGH	    (0x0f)	/* PS数据高8bit     */

#define   ALS_CONFIG              (0x10)
#define   ALS_CALIBRATION         (0x19)
#define   ALS_LOW_THRESHOLD_7_0   (0x1a)
#define   ALS_LOW_THRESHOLD_15_8  (0x1b)
#define   ALS_HIGH_THRESHOLD_7_0  (0x1c)
#define   ALS_HIGH_THRESHOLD_15_8 (0x1d)

#define   PS_CONFIG               (0x20)
#define   PS_LED_DRIVER           (0x21)
#define   PS_INT_FORM             (0x22)
#define   PS_MEAN_TIME            (0x23)
#define   PS_LED_WATTING_TIME     (0x24)
#define   PS_CALIBRATION_L        (0x28)
#define   PS_CALIBRATION_H        (0x29)
#define   PS_LOW_THRESHOLD_2_0    (0x2a)
#define   PS_LOW_THRESHOLD_10_3   (0x2b)
#define   PS_HIGH_THRESHOLD_2_0   (0x2c)
#define   PS_HIGH_THRESHOLD_10_3  (0x2d)



#endif

