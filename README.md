## Man-in-the-Middle Attack against Object Detection

> This project uses a Raspberry Pi 4 to fool the object detection system in real-time.

[[ Talk ]](https://minm.wuhanstudio.uk) [[ Video ]](https://youtu.be/OvIpe-R3ZS8) [[ Paper ]](https://arxiv.org/abs/2208.07174) [[ Code ]](https://github.com/wuhanstudio/adversarial-camera)

We use a Raspberry Pi 4 to eavesdrop and manipulate the video stream transferred over the USB cable.

![](doc/demo.png)

The linux program that injects the perturbation and simulates a virtual USB camera can be found here: [uvc-gadget](https://github.com/wuhanstudio/adversarial-camera/tree/master/hardware/buildroot-external-raspi4/package/uvc-gadget/src).

![](doc/demo.jpg)

The Man-in-the-Middle Attack consists of two steps:

- Step 1: [Generating the perturbation](detection/README.md).
- Step 2: [Deploying the perturbation](hardware/README.md).



### Quick Start

We’ve released pre-built images for Raspberry Pi 4 that can be flashed to microSD cards:

- For development: **raspbian_minm_attack.img**
- For release: **buildroot_minm_attack.img**

Checkout the [latest release](https://github.com/wuhanstudio/adversarial-camera/releases).

<br />

## WHite-box Adversarial Toolbox (WHAT)

<!-- [![CircleCI](https://circleci.com/gh/wuhanstudio/whitebox-adversarial-toolbox.svg?style=svg)](https://circleci.com/gh/wuhanstudio/whitebox-adversarial-toolbox) -->
[![Build Status](https://app.travis-ci.com/wuhanstudio/whitebox-adversarial-toolbox.svg?branch=master)](https://app.travis-ci.com/wuhanstudio/whitebox-adversarial-toolbox)
[![PyPI version](https://badge.fury.io/py/whitebox-adversarial-toolbox.svg)](https://badge.fury.io/py/whitebox-adversarial-toolbox)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/whitebox-adversarial-toolbox)](https://pypi.org/project/whitebox-adversarial-toolbox/)
[![](https://img.shields.io/badge/Documentation-brightgreen)](https://what.wuhanstudio.uk/)

Alternatively, you can try real-time white-box attacks using our toolbox.

### Installation

```python
pip install whitebox-adversarial-toolbox
```

<a href="https://github.com/wuhanstudio/whitebox-adversarial-toolbox"><img src="https://camo.githubusercontent.com/1aa1ac6b346540aa672c2f89fe93dc2e23ee478331fe9ad0f1c26d527fcdad8f/68747470733a2f2f776861742e777568616e73747564696f2e756b2f696d616765732f776861742e706e67" width=35%></a>

### Usage (CLI)

```
Usage: what [OPTIONS] COMMAND [ARGS]...

  The CLI tool for WHitebox-box Adversarial Toolbox (what).

Options:
  --help  Show this message and exit.

Commands:
  attack   Manage Attacks
  example  Manage Examples
  model    Manage Deep Learning Models
```
