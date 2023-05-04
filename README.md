## Man-in-the-Middle Attack against Object Detection

> Is Deep Learning secure for Robots?

[[ Talk ]](https://minm.wuhanstudio.uk) [[ Video ]](https://youtu.be/OvIpe-R3ZS8) [[ Paper ]](https://arxiv.org/abs/2208.07174) [[ Code ]](https://github.com/wuhanstudio/adversarial-camera)


This project uses a Raspberry Pi 4 to fool the object detection system in real-time. 

![](doc/demo.jpg)

It's no more a secret that deep neural networks are vulnerable to adversarial attacks. Does this mean deep-learning-enabled object detection systems are no more secure for safety-critical applications? We use a Raspberry pi 4 to eavesdrop and manipulate the image transferred over the USB cable.

The Man-in-the-Middle Attack consists of two steps:

- Step 1: Generate the perturbation.

- Step 2: Deploy the perturbation.

The first step generates a Universal Adversarial Perturbation (UAP) that can fool an object detection model to misclassify all the input images in a dataset.

The second step deploys the UAP using a Raspberry Pi 4 (or any hardware that can simulate a virtual USB Camera).



### Step 1: Generate the perturbation

In the `detection` folder, you can find instructions that help you set up a real-time object detection system, and then generates the Universal Adversarial Perturbation (UAP) to fool the object detection model.

Alternatively, you can generate the UAP using the [WHite-box Adversarial Toolbox (WHAT)](https://github.com/wuhanstudio/whitebox-adversarial-toolbox).

<img src="https://camo.githubusercontent.com/1aa1ac6b346540aa672c2f89fe93dc2e23ee478331fe9ad0f1c26d527fcdad8f/68747470733a2f2f776861742e777568616e73747564696f2e756b2f696d616765732f776861742e706e67" width="30%" />

You can also skip this step, and use our pre-generated UAP for the deployment.




### Step 2: Deploy the perturbation

The `hardware` folder contains instructions that help you deploy the perturbation using a Raspberry Pi 4.

![](doc/demo.png)



### Citations

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

