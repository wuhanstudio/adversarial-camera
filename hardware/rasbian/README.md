## Adversarial Camera

**Important: Please use [2021-10-30-raspios-bullseye-armhf.img](https://downloads.raspberrypi.org/raspios_armhf/images/raspios_armhf-2021-11-08/)**

There is a known issue on the latest version of raspi os:

```
UVC: Possible USB shutdown requested from Host, seen during VIDIOC_DQBUF
select timeout
UVC: Stopping video stream.
```



We can use a Raspberry Pi 4 to deploy the perturbation and simulate an adversarial camera to fool the object detection system.

![](../../doc/demo.png)
![](../../doc/filter.jpg)



### Step 1: Set up the Operating System

Editing the `/boot/cmdline.txt` to enable the USB device mode:

```
modules-load=dwc2,libcomposite
```

And add an extra line at the end of `/boot/config.txt`

```
dtoverlay=dwc2
```



### Step 2: Compile the program

Now you are ready to compile the program:

```
$ git clone https://github.com/wuhanstudio/adversarial-camera && cd adversarial-camera/hardware/rasbian
$ sudo cp configfs/piwebcam.service configfs/piwebcam configfs/usb-gadget.sh /etc/systemd/system/
$ sudo systemctl enable piwebcam
$ cd uvc-gadget && make
```

If everything works fine,  **reboot** the system with a USB camera plugged in. You should see several video devices:

```
pi@raspberrypi:~ $ v4l2-ctl --list-device
fe980000.usb (gadget):
        /dev/video0

USB2.0 PC CAMERA: USB2.0 PC CAM (usb-0000:01:00.0-1.4):
        /dev/video1
        /dev/video2
```

Now you can fake a USB camera:

```
$ sudo ./uvc-gadget/uvc-gadget -u /dev/video0 -v /dev/video1 -f 1 -r 1
```

**Cheers!** If the raspberry pi is connected to a Windows PC, you should see a new camera device from device manager.

![](../../doc/device.png)
