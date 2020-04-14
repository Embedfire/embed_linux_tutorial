import serial
from time import sleep

def recv(serial):
    while True:
        data = serial.read_all()
        if data != b'' :
            break
        
    return data.decode()


if __name__ == '__main__':
    serial = serial.Serial('/dev/tty10', 115200)  # /dev/ttymxc2
    if serial.isOpen() :
        print("open success")
    else :
        print("open failed")

    while True:
        data =recv(serial)
        if data != '' :
            print("receive:",data)
            serial.write(data.encode()) # 数据回显


