> Please use pre-built images if you don't want to build the system from scratch.

We’ve released pre-built images for Raspberry Pi 4 that can be flashed to microSD cards:

- For development: **rasbian_minm_attack.img**
- For release: **buildroot_minm_attack.img**

Checkout the [latest release](https://github.com/wuhanstudio/adversarial-camera/releases).



## Step 2: Deploying the perturbation (Rasbian)

Please follow instructions in the [rasbian](rasbian) foler.




## Step 2: Deploying the perturbation (Buildroot)

```
# Clone the repo
$ git clone https://github.com/wuhanstudio/adversarial-camera/
$ cd adversarial-camera

# Inilialize buildroot submodule
$ git submodule init
$ git submodule update

# Apply the patch to use different versions of OpenCV and rpi-firmware
$ cd hardware/buildroot
$ git apply ../buildroot_rpi_firmware_opencv4.patch

# Compile Buildroot
$ make BR2_EXTERNAL=../buildroot-external-raspi4/ raspi4_minm_attack_defconfig
$ make BR2_EXTERNAL=../buildroot-external-raspi4/
```

After compiling, the image is available in `output/images/sdcard.img` that can be flashed to a SD card using:

```
$ sudo dd if=output/images/sdcard.img of=/dev/sdb
```

**With a USB camera plugged in before powering up**, the `uvc-gadget` will automatically initiates the attack.
