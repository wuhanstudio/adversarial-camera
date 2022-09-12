## Adversarial Detection

> Real-time adversarial attacks against Object Detection.

![](../doc/web.png)

![](../doc/filter.jpg)

### Quick Start

You may use [anaconda](https://www.continuum.io/downloads) or [miniconda](https://conda.io/miniconda.html). 

```
$ # CPU
$ conda env create -f environment.yml
$ conda activate adversarial-camera

$ # GPU
$ conda env create -f environment_gpu.yml
$ conda activate adversarial-gpu-camera

$ python detect.py --model model/yolov3-tiny.h5 --class_name coco_classes.txt
```

The web page will be available at: http://localhost:9090/

That's it!
