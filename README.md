## Man-in-the-Middle Attack against Object Detection

> This project uses a Raspberry Pi 4 to fool the object detection system in real-time.

[[ Talk ]](https://minm.wuhanstudio.uk) [[ Video ]](https://youtu.be/OvIpe-R3ZS8) [[ Paper ]](https://arxiv.org/abs/2208.07174) [[ Code ]](https://github.com/wuhanstudio/adversarial-camera)

We use a Raspberry Pi 4 to eavesdrop and manipulate the video stream transferred over the USB cable.

![](doc/demo.png)

![](doc/demo.jpg)

The Man-in-the-Middle Attack consists of two steps:

- Step 1: [Generating the perturbation](detection/README.md).
- Step 2: [Deploying the perturbation](hardware/README.md).



### Quick Start

We’ve released pre-built images for Raspberry Pi 4 that can be flashed to microSD cards:

- For development: **raspbian_minm_attack.img**
- For release: **buildroot_minm_attack.img**

Checkout the [latest release](https://github.com/wuhanstudio/adversarial-camera/releases).

## WHite-box Adversarial Toolbox (WHAT)

<!-- [![CircleCI](https://circleci.com/gh/wuhanstudio/whitebox-adversarial-toolbox.svg?style=svg)](https://circleci.com/gh/wuhanstudio/whitebox-adversarial-toolbox) -->
[![Build Status](https://app.travis-ci.com/wuhanstudio/whitebox-adversarial-toolbox.svg?branch=master)](https://app.travis-ci.com/wuhanstudio/whitebox-adversarial-toolbox)
[![PyPI version](https://badge.fury.io/py/whitebox-adversarial-toolbox.svg)](https://badge.fury.io/py/whitebox-adversarial-toolbox)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/whitebox-adversarial-toolbox)](https://pypi.org/project/whitebox-adversarial-toolbox/)

Alternatively, you can try real-time white-box attacks using our toolbox.

### Installation

```python
pip install whitebox-adversarial-toolbox
```

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

## Citations

```
@misc{han2022minm,
  doi = {10.48550/ARXIV.2208.07174},
  url = {https://arxiv.org/abs/2208.07174},
  author = {Wu, Han and Rowlands, Sareh and Wahlstrom, Johan},
  title = {A Man-in-the-Middle Attack against Object Detection Systems},
  publisher = {arXiv},
  year = {2022}
}
```

