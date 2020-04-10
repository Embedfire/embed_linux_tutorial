#! /bin/sh

type devscan

if [ $? -eq 0 ]; then
    eventx=$(devscan "goodix-ts")
    echo "eventx=$eventx"
    if [ "$eventx " != " " ]; then
        if [ ! -f "/etc/pointercal" ]; then
            type ts_calibrate
            if [ $? -eq 0 ]; then
                ts_calibrate
            fi
        fi
    else 
        echo "eventx is null"
    fi
else
    echo "please install devscan"
    echo
    echo "sudo apt-get install devscan"
    exit
fi

export TSLIB_TSDEVICE=/dev/input/$eventx
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_CALIBFILE=/etc/pointercal
export QT_QPA_GENERIC_PLUGINS=tslib:/dev/input/$eventx
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS=/dev/input/$eventx

echo "start pyqt pyqt5_demo..."
python3 pyqt5_demo.py
