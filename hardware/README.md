## Adversarial Camera

We can use a Raspberry Pi 4 to deploy the perturbation and simulate an adversarial camera to fool the object detection system.

![](../doc/demo.png)
![](../doc/filter.jpg)

### Step 1: Set up the Operating System

**Important: Please use [2021-10-30-raspios-bullseye-armhf.img](https://downloads.raspberrypi.org/raspios_armhf/images/raspios_armhf-2021-11-08/)**

There is a known issue on the latest version of raspios:

```
UVC: Possible USB shutdown requested from Host, seen during VIDIOC_DQBUF
select timeout
UVC: Stopping video stream.
```

Editing the `/boot/cmdline.txt` to enable the OTG mode:

```
modules-load=dwc2,libcomposite
```

And add an extra line at the end of `/boot/config.txt`

```
dtoverlay=dwc2
```

Enable the configfs kernel module by adding this to `/etc/modules`:

```
libcomposite
```

<!-- We also need to enable the v4l2loopback kernel module for a v4l2 dummy device for testing purpose -->

<!-- sudo modprobe v4l2loopback devices=2 video_nr="40" max_buffers="8" exclusive_caps="1" -->



### Step 2: Compile the program

Now you are ready to compile the program:

```
# Clone to home directory /home/pi
$ git clone https://github.com/wuhanstudio/adversarial-camera & cd adversarial-camera/hardware
$ sudo cp configfs/piwebcam.service configfs/piwebcam configfs/usb-gadget.sh /etc/systemd/system/
$ sudo systemctl enable piwebcam
$ cd uvc-gadget && make
```

```
# gst-launch-1.0 videotestsrc ! v4l2sink device=/dev/video40
# gst-launch-1.0 videotestsrc ! "video/x-raw, width=640, height=360, fps=30/1" ! avenc_mjpeg !  v4l2sink device=/dev/video42
```

If everything works fine, after rebooting you should see:

```
pi@raspberrypi:~ $ v4l2-ctl --list-device
fe980000.usb (gadget):
        /dev/video0

bcm2835-codec-decode (platform:bcm2835-codec):
        /dev/video10
        /dev/video11
        /dev/video12
        /dev/video18

bcm2835-isp (platform:bcm2835-isp):
        /dev/video13
        /dev/video14
        /dev/video15
        /dev/video16

USB2.0 PC CAMERA: USB2.0 PC CAM (usb-0000:01:00.0-1.4):
        /dev/video1
        /dev/video2

Dummy video device (0x0000) (platform:v4l2loopback-000):
        /dev/video40
```

Now you can fake a USB camera:

```
$ sudo ./uvc-gadget/uvc-gadget -f 1 -r 1 -u /dev/video0 -v /dev/video1
```

## How to use

    Usage: ./uvc-gadget [options]
    
    Available options are
        -f <format> Select frame format
                0 = V4L2_PIX_FMT_YUYV
                1 = V4L2_PIX_FMT_MJPEG
        -r <resolution> Select frame resolution:
                0 = 360p, VGA (640x360)
                1 = 720p, WXGA (1280x720)
        -u device      UVC Video Output device
        -v device      V4L2 Video Capture device
        -h help        Print this help screen and exit

## Change log

- Apply patchset [Bugfixes for UVC gadget test application](https://www.spinics.net/lists/linux-usb/msg99220.html)  

- Apply patchset [UVC gadget test application enhancements](https://www.spinics.net/lists/linux-usb/msg84376.html)  

- Add Readme/.gitignore and documentations  
  Copy linux-3.18.y/drivers/usb/gadget/function/uvc.h into repository, change include path for build
