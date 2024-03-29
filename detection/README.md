# Step 1: Generating the Perturbation

We first need to generate an image-agnostic Universal Adversarial Perturbation (UAP).

## Quick Start

You may use [anaconda](https://www.continuum.io/downloads) or [miniconda](https://conda.io/miniconda.html). 

```
# Clone the repo
$ git clone https://github.com/wuhanstudio/adversarial-camera/
$ cd adversarial-camera/detection

# For CPU
$ conda env create -f environment.yml
$ conda activate adversarial-camera

# For GPU
$ conda env create -f environment_gpu.yml
$ conda activate adversarial-gpu-camera

$ python detect.py --model model/yolov3-tiny.h5 --class_name coco_classes.txt
```

The web page will be available at: http://localhost:9090/

That's it! The perturbation is saved as `noise.npy` file in the `detection/noise/` folder.

<img src="../doc/web.png" width=80%>
<img src="../doc/filter.jpg" width=80%>

## White-box Adversarial Toolbox

Alternatively, you can generate the UAP using the [WHite-box Adversarial Toolbox (WHAT)](https://github.com/wuhanstudio/whitebox-adversarial-toolbox).

<img src="https://camo.githubusercontent.com/1aa1ac6b346540aa672c2f89fe93dc2e23ee478331fe9ad0f1c26d527fcdad8f/68747470733a2f2f776861742e777568616e73747564696f2e756b2f696d616765732f776861742e706e67" width=20%>

<br />

# Step 2: Deploying the Perturbation

You can use a raspberry pi 4 to deploy the perturbation ([documentation](../hardware/README.md)).
