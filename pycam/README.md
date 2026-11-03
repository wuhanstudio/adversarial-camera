
```
sudo apt install v4l-utils
```

```
sudo modprobe v4l2loopback video_nr="40,41"

v4l2-ctl --list-devices
v4l2-ctl --list-formats-ext -d /dev/video0
```
