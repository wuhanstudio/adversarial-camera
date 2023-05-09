#!/bin/bash

# sudo modprobe v4l2loopback devices=2 video_nr="40" max_buffers="8" exclusive_caps="1"

sudo /etc/piwebcam/usb-gadget.sh

/usr/bin/v4l2-ctl -c auto_exposure=0
/usr/bin/v4l2-ctl -c auto_exposure_bias=8
/usr/bin/v4l2-ctl -c contrast=20
/usr/bin/v4l2-ctl -c video_bitrate=25000000

sudo uvc-gadget -f1 -r1 -u /dev/video2 -v /dev/video0 -n noise.npy
