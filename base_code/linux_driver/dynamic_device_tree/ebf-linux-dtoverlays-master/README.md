# 内核配置

## dth11/dsb18b20——单总线设备驱动
 ->Device Drivers
->Dallas's 1-wire support<*>



## WM8960
-> Device Drivers 
  -> Sound card support 
    -> Advanced Linux Sound Architecture
        -> ALSA for SoC audio support 
            -> CODEC drivers
                -> Wolfson Microelectronics WM8960 CODEC< >

PS：使用声卡驱动是需要配置声卡

## 摄像头
  ->Device Drivers 
   -> Multimedia support 
    -> V4L platform devices-
      -> MXC Camera/V4L2 PRP Features support
        -> OmniVision ov5640 camera support<M> 
        ->OmniVision ov5640 camera support < > 


## AP6236
  -> Device Drivers               
    -> Network device support                                         
       -> Wireless LAN
        <*>   Broadcom FullMAC wireless cards support                        
                (/lib/firmware/bcm/AP6236/Wi-Fi/fw_bcm43436b0.bin) Firmware path     
                (/lib/firmware/bcm/AP6236/Wi-Fi/nvram_ap6236.txt) NVRAM path   

需要修改net/wireless/文件夹下的Kconfig和Makefile，并且需要把wifi固件放到对应的文件夹



