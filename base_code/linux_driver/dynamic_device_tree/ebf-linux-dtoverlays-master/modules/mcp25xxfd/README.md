# CAN-HAT


The drivers of [Raspberry pi  CAN HAT](https://www.seeedstudio.com/2-Channel-CAN-BUS-FD-Shield-for-Raspberry-Pi-p-4072.html) for Raspberry Pi.

### Install CAN-HAT
Get the CAN-HAT  source code. and install all linux kernel drivers
```bash
git clone https://github.com/seeed-Studio/pi-hats
cd pi-hats/CAN-HAT
sudo ./install.sh 
sudo reboot
```

## Raspberry pi  CAN HAT

[![](https://github.com/SeeedDocument/2-Channel-CAN-BUS-FD-Shield-for-Raspberry-Pi/raw/master/img/block.jpg)](https://www.seeedstudio.com/2-Channel-CAN-BUS-FD-Shield-for-Raspberry-Pi-p-4072.html)


Check the kernel log to see if MCP2517 was initialized successfully.You will also see can0 and can1 appear in the list of ifconfig results

```bash
pi@raspberrypi:~ $ dmesg | grep spi
[    3.725586] mcp25xxfd spi0.0 can0: MCP2517 successfully initialized.
[    3.757376] mcp25xxfd spi1.0 can1: MCP2517 successfully initialized.

pi@raspberrypi:~/pi-hats/CAN-HAT $ ifconfig -a
can0: flags=128<NOARP>  mtu 16
        unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

can1: flags=128<NOARP>  mtu 16
        unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether b8:27:eb:c7:ed:4f  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

wlan0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.250.42  netmask 255.255.255.0  broadcast 192.168.250.255
        inet6 fe80::3842:7323:7c0d:f6d2  prefixlen 64  scopeid 0x20<link>
        ether b8:27:eb:92:b8:1a  txqueuelen 1000  (Ethernet)
        RX packets 2654  bytes 249303 (243.4 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 4433  bytes 4765896 (4.5 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

```




#### Next step
Set the can fd protocol, and the dbitrate can be set to 8M speed. [Refer to the kernel documentation for more usage](https://www.kernel.org/doc/Documentation/networking/can.txt)

```bash
sudo ip link set can0 up type can bitrate 1000000   dbitrate 8000000 restart-ms 1000 berr-reporting on fd on
sudo ip link set can1 up type can bitrate 1000000   dbitrate 8000000 restart-ms 1000 berr-reporting on fd on

sudo ifconfig can0 txqueuelen 65536
sudo ifconfig can1 txqueuelen 65536
```

The hardware is wired to can0 and can1 interface.

0_L  <===> 1_L

0_H  <===> 1_H

Open two terminal windows and enter the following commands in the Windows to test can fd protocol.
```bash
#send data
cangen can0 -mv 
```

```bash
#dump data
candump can0 
```
#### Communicate with [CAN_BUS_Shield](https://www.seeedstudio.com/CAN-BUS-Shield-V2-p-2921.html)
![](https://github.com/Seeed-Studio/pi-hats/raw/master/images/can_hat_and_arduinno_hardware.jpg) 

1. CAN_BUS_Shield send and CAN HAT receive

Arduino code:
```c
// demo: CAN-BUS Shield, send data
// loovee@seeed.cc

#include <mcp_can.h>
#include <SPI.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
}

unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
void loop()
{
    //send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    stmp[7] = stmp[7]+1;
    if(stmp[7] == 100)
    {
        stmp[7] = 0;
        stmp[6] = stmp[6] + 1;
        
        if(stmp[6] == 100)
        {
            stmp[6] = 0;
            stmp[5] = stmp[6] + 1;
        }
    }
    
    CAN.sendMsgBuf(0x00, 0, 8, stmp);
    delay(100);                       // send data per 100ms
}
// END FILE
```
Respberry pi setting and and use can-util to receive
```bash
#set 500k baudrate
pi@raspberrypi:~ $ sudo ip link set can0 up type can bitrate 500000
pi@raspberrypi:~ $ ip -details link show can0
3: can0: <NOARP,UP,LOWER_UP,ECHO> mtu 16 qdisc pfifo_fast state UNKNOWN mode DEFAULT group default qlen 10
    link/can  promiscuity 0 
    can state ERROR-ACTIVE (berr-counter tx 0 rx 0) restart-ms 0 
	  bitrate 500000 sample-point 0.875 
	  tq 25 prop-seg 34 phase-seg1 35 phase-seg2 10 sjw 1
	  mcp25xxfd: tseg1 2..256 tseg2 1..128 sjw 1..128 brp 1..256 brp-inc 1
	  mcp25xxfd: dtseg1 1..32 dtseg2 1..16 dsjw 1..16 dbrp 1..256 dbrp-inc 1
	  clock 40000000numtxqueues 1 numrxqueues 1 gso_max_size 65536 gso_max_segs 65535 
#receive
pi@raspberrypi:~ $ candump can0
  can0  000   [8]  00 00 00 00 00 00 00 05
  can0  000   [8]  00 00 00 00 00 00 00 06
  can0  000   [8]  00 00 00 00 00 00 00 07
  can0  000   [8]  00 00 00 00 00 00 00 08
  can0  000   [8]  00 00 00 00 00 00 00 09
  can0  000   [8]  00 00 00 00 00 00 00 0A
  can0  000   [8]  00 00 00 00 00 00 00 0B
  can0  000   [8]  00 00 00 00 00 00 00 0C
  can0  000   [8]  00 00 00 00 00 00 00 0D
  can0  000   [8]  00 00 00 00 00 00 00 0E
  can0  000   [8]  00 00 00 00 00 00 00 0F
  can0  000   [8]  00 00 00 00 00 00 00 10
  can0  000   [8]  00 00 00 00 00 00 00 11
  can0  000   [8]  00 00 00 00 00 00 00 12
  can0  000   [8]  00 00 00 00 00 00 00 13
  can0  000   [8]  00 00 00 00 00 00 00 14
  can0  000   [8]  00 00 00 00 00 00 00 15
  can0  000   [8]  00 00 00 00 00 00 00 16
  can0  000   [8]  00 00 00 00 00 00 00 17
  can0  000   [8]  00 00 00 00 00 00 00 18
  can0  000   [8]  00 00 00 00 00 00 00 19
  can0  000   [8]  00 00 00 00 00 00 00 1A
  can0  000   [8]  00 00 00 00 00 00 00 1B
  can0  000   [8]  00 00 00 00 00 00 00 1C
  can0  000   [8]  00 00 00 00 00 00 00 1D
```
Respberry pi use python code to receive
```bash
# install python-can
sudo pip3 install python-can
```
run following python code
```python
import can

can_interface = 'can0'
bus = can.interface.Bus(can_interface, bustype='socketcan_native')
while True:
    message = bus.recv(1.0) # Timeout in seconds.
    if message is None:
            print('Timeout occurred, no message.')
    print(message)
```
python code run result
```bash
pi@raspberrypi:~ $ python3 can_test.py   
Timestamp: 1550471771.628215        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0e 63     Channel: can0
Timestamp: 1550471772.629302        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 00     Channel: can0
Timestamp: 1550471773.630658        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 01     Channel: can0
Timestamp: 1550471774.632018        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 02     Channel: can0
Timestamp: 1550471775.633395        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 03     Channel: can0
Timestamp: 1550471776.634774        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 04     Channel: can0
Timestamp: 1550471777.636135        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 05     Channel: can0
Timestamp: 1550471778.637481        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 06     Channel: can0
Timestamp: 1550471779.638859        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 07     Channel: can0
Timestamp: 1550471780.640222        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 08     Channel: can0
Timestamp: 1550471781.641602        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 09     Channel: can0
Timestamp: 1550471782.642970        ID: 0000    S                DLC:  8    00 00 00 00 00 00 0f 0a     Channel: can0

```

2. CAN_BUS_Shield receive and CAN HAT send

raspberry pi use cangen to send random package 
```bash
pi@raspberrypi:~ $ cangen can0 -v 
  can0  442#14.C4.1A.1A.C2.25.79.25
  can0  748#4E.C7.8B.0B.6E.B9.15.77
  can0  1E4#64.D4.62.22.2F.A6.BF
  can0  1DD#69.6F.61.33.1F.59.E4.7C
  can0  63D#
  can0  764#2C.C1.E3
  can0  68B#11.9C.63.6D.EA.E9.4B
  can0  329#DA.06.2C.34.6C
  can0  7DD#2E.F5.E0.2A.26.77.58.38
  can0  1BE#94.30.6E.2F.A2.7B.E3.1D
  can0  654#D1.21.A3.58.31.E8.51.5F
  can0  706#51.41.36.5C.43.8D.AE.5D
  can0  34A#89.F2.DE.33.AE.52.38.6C
  can0  6AC#C1.35.83.41.37
  can0  38C#22.AF
  can0  208#22.8E.97.58.E5.69.F7.2C
```
Arduino receive code:
```c
// demo: CAN-BUS Shield, receive data with interrupt mode
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp_can.h"

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin


unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");

    attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
}

void MCP2515_ISR()
{
    flagRecv = 1;
}

void loop()
{
    if(flagRecv) 
    {                                   // check if get data

        flagRecv = 0;                   // clear flag

        // iterate over all pending messages
        // If either the bus is saturated or the MCU is busy,
        // both RX buffers may be in use and reading a single
        // message does not clear the IRQ conditon.
        while (CAN_MSGAVAIL == CAN.checkReceive()) 
        {
            // read data,  len: data length, buf: data buf
            CAN.readMsgBuf(&len, buf);

            // print the data
            for(int i = 0; i<len; i++)
            {
                Serial.print(buf[i]);Serial.print("\t");
            }
            Serial.println();
        }
    }
}
```
Arduino receive data
![](https://github.com/Seeed-Studio/pi-hats/raw/master/images/arduino_receive.png) 
You alse can use python-can to send data
```python
import time
import can

bustype = 'socketcan_native'
channel = 'can0'

def producer(id):
    """:param id: Spam the bus with messages including the data id."""
    bus = can.interface.Bus(channel=channel, bustype=bustype)
    for i in range(10):
        msg = can.Message(arbitration_id=0xc0ffee, data=[id, i, 0, 1, 3, 1, 4, 1], extended_id=False)
        bus.send(msg)
    # Issue #3: Need to keep running to ensure the writing threads stay alive. ?
    time.sleep(1)

producer(10)

```
### uninstall CAN-HAT

```
pi@raspberrypi:~/pi-hats/CAN-HAT $ sudo ./uninstall.sh 
...
------------------------------------------------------
Please reboot your raspberry pi to apply all settings
Thank you!
------------------------------------------------------
```


Enjoy !
