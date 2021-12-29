#!/bin/bash
mkdir /sys/kernel/config/usb_gadget/pi4

echo 0x1d6b > /sys/kernel/config/usb_gadget/pi4/idVendor
echo 0x0104 > /sys/kernel/config/usb_gadget/pi4/idProduct
echo 0x0100 > /sys/kernel/config/usb_gadget/pi4/bcdDevice
echo 0x0200 > /sys/kernel/config/usb_gadget/pi4/bcdUSB

echo 0xEF > /sys/kernel/config/usb_gadget/pi4/bDeviceClass
echo 0x02 > /sys/kernel/config/usb_gadget/pi4/bDeviceSubClass
echo 0x01 > /sys/kernel/config/usb_gadget/pi4/bDeviceProtocol

mkdir /sys/kernel/config/usb_gadget/pi4/strings/0x409
echo 100000000d2386db > /sys/kernel/config/usb_gadget/pi4/strings/0x409/serialnumber
echo "Samsung" > /sys/kernel/config/usb_gadget/pi4/strings/0x409/manufacturer
echo "PI4 USB Device" > /sys/kernel/config/usb_gadget/pi4/strings/0x409/product
mkdir /sys/kernel/config/usb_gadget/pi4/configs/c.2
mkdir /sys/kernel/config/usb_gadget/pi4/configs/c.2/strings/0x409
echo 500 > /sys/kernel/config/usb_gadget/pi4/configs/c.2/MaxPower
echo "UVC" > /sys/kernel/config/usb_gadget/pi4/configs/c.2/strings/0x409/configuration

mkdir /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0
mkdir /sys/kernel/config/usb_gadget/pi4/functions/acm.usb0
mkdir -p /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/control/header/h
ln -s /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/control/header/h /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/control/class/fs

mkdir -p /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/mjpeg/m/720p
cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/mjpeg/m/720p/dwFrameInterval
5000000
EOF
cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/mjpeg/m/720p/wWidth
1280
EOF
cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/mjpeg/m/720p/wHeight
720
EOF
cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/mjpeg/m/720p/dwMinBitRate
29491200
EOF
cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/mjpeg/m/720p/dwMaxBitRate
29491200
EOF
cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/mjpeg/m/720p/dwMaxVideoFrameBufferSize
1843200
EOF

#mkdir -p /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/uncompressed/u/720p
#cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/uncompressed/u/720p/dwFrameInterval
#5000000
#EOF
#cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/uncompressed/u/720p/wWidth
#1280
#EOF
#cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/uncompressed/u/720p/wHeight
#720
#EOF
#cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/uncompressed/u/720p/dwMinBitRate
#29491200
#EOF
#cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/uncompressed/u/720p/dwMaxBitRate
#29491200
#EOF
#cat <<EOF > /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/uncompressed/u/720p/dwMaxVideoFrameBufferSize
#1843200
#EOF

mkdir /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/header/h
cd /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0/streaming/header/h
ln -s ../../mjpeg/m
#ln -s ../../uncompressed/u
cd ../../class/fs
ln -s ../../header/h
cd ../../class/hs
ln -s ../../header/h
cd ../../../../..

ln -s /sys/kernel/config/usb_gadget/pi4/functions/uvc.usb0 /sys/kernel/config/usb_gadget/pi4/configs/c.2/uvc.usb0
ln -s /sys/kernel/config/usb_gadget/pi4/functions/acm.usb0 /sys/kernel/config/usb_gadget/pi4/configs/c.2/acm.usb0
udevadm settle -t 5 || :
ls /sys/class/udc > /sys/kernel/config/usb_gadget/pi4/UDC

