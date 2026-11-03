import cv2
import colorsys
import numpy as np
import pyvirtualcam

from resize import bilinear_resize

height, width = 720, 1280  # Default resolution for the virtual camera

noise = np.load("noise.npy")  # Load the noise array from the .npy file
if noise is not None:
    print(f"Noise shape: {noise.shape}, Image shape: {height}x{width}")
    noise_r = bilinear_resize(noise[:, :, 0], height, width)
    noise_g = bilinear_resize(noise[:, :, 1], height, width)
    noise_b = bilinear_resize(noise[:, :, 2], height, width)
    noise_l = np.dstack((noise_r, noise_g, noise_b))

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

if not cap.isOpened():
    print("Error: Cannot open camera index 0")
    raise SystemExit(1)

with pyvirtualcam.Camera(width=width, height=height, fps=20) as cam:
    print(f'Using virtual camera: {cam.device}')

    while True:
        ret, frame = cap.read()

        if frame is None:
            continue

        # Image preprocessing
        image = cv2.resize(frame, (width, height))
        image = np.array(image).astype(np.float32) / 255.0
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

        if noise is not None:
            # Add the perturbation
            image = image + noise_l
            image = np.clip(image, 0, 1)

        image = cv2.resize(image, (cam.width, cam.height))
        image = (image * 255).astype(np.uint8)

        cam.send(image)
        cam.sleep_until_next_frame()
