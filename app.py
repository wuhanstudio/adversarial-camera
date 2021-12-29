#! /usr/bin/env python3
import os
import sys
import time
import queue
import argparse
import cv2
import pafy
import numpy as np
import tensorflow as tf
from threading import Thread, Event
from logging import basicConfig, getLogger
from PIL import Image

# Set up logger
basicConfig()
logger = getLogger(__name__)
logger.setLevel(os.environ.get("LOG_LEVEL", "INFO"))

def mask_image_generator(is_running, estimator, input_q, output_q):
    while is_running.is_set():
        image = input_q.get()
        image_height, image_width, _ = image.shape

        # Perform preprocess
        #start = time.time()
        # input_data = estimator.preprocess_input_image(image)
        #duration = time.time() - start
        #logger.debug(f"Preprocess took {duration: .3f} s")

        # Run the estimator
        #start = time.time()
        #result = estimator.estimate(input_data)
        #duration = time.time() - start
        #logger.debug(
        #    f"Estimation took {duration: .3f} s ({1/duration: .3f} fps)")

        # Generate the mask image to composite
        # mask = estimator.generate_mask(result["segments"])
        # mask_img = Image.fromarray(mask * 255)
        # mask_img = Image.fromarray(input_data)
        mask_img = Image.fromarray(image)
        mask_img = mask_img.resize(
            (image_width, image_height), Image.BICUBIC).convert("RGB")
        mask_img = tf.keras.preprocessing.image.img_to_array(
            mask_img, dtype=np.uint8)

        # Add to the queue
        if output_q.full():
            output_q.get()
        output_q.put_nowait(mask_img)


def main(ARGS):
    # Init variables
    latest_mask_img = None
    cap_file = None
    input_q = queue.Queue(maxsize=1)
    output_q = queue.Queue(maxsize=1)
    estimator = None 

    # Open the capture device
    cap = cv2.VideoCapture(ARGS.cap_source)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, ARGS.cap_res[0])
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, ARGS.cap_res[1])

    if not cap.isOpened():
        logger.error("Could not open the specified device.")
        return

    # Load the background image/video
    if ARGS.bg_video:
        cap_file = cv2.VideoCapture(ARGS.bg_video)
    elif ARGS.bg_url:
        v_pafy = pafy.new(ARGS.bg_url)
        play = v_pafy.getbestvideo()
        cap_file = cv2.VideoCapture(play.url)

    # Start the prediction thread
    is_running = Event()
    is_running.set()

    th = Thread(name="mask_image_generator", target=mask_image_generator,
                args=(is_running, estimator, input_q, output_q,))
    th.isDaemon = True
    th.start()

    while True:
        ret, frame = cap.read()
        if not ret:
            logger.warn("cap.read() failed.")
            continue

        # Flip the image if requested
        if ARGS.flip != -1:
            frame = cv2.flip(frame, ARGS.flip)

        # Pass the image to the generator thread
        im_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        if input_q.full():
            input_q.get()
        input_q.put(im_rgb, block=False)

        # Get the mask image if possible
        if output_q.qsize() > 0:
            latest_mask_img = output_q.get_nowait()

        if latest_mask_img is not None:
            # Composite the foreground and background images
            mask = latest_mask_img
            composed = np.zeros(frame.shape, dtype=frame.dtype)
            for i in range(3):
                composed[:,:,i] = frame[:,:,i] # / 255

            # Output the frame
            if ARGS.show_only_gui:
                cv2.imshow("frame", composed)
            else:
                sys.stdout.buffer.write(composed.tobytes())
        else:
            if ARGS.show_only_gui:
                cv2.imshow("frame", frame)
            else:
                sys.stdout.buffer.write(frame.tobytes())

        if cv2.waitKey(1) != -1:
            break

    is_running.clear()
    th.join()
    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    # Init argument parser
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description='Streaming video to stdout with applying special effects. \
            Use ffmpeg to write the stream to another video device.')
    parser.add_argument('-c', '--cap-source', metavar='CAMERA_SOURCE',
                        type=int, default=0, help='V4L2 Camera source id.')
    parser.add_argument('-r', '--cap-res', metavar='WH',
                        type=int, nargs=2, default=(640, 480),
                        help='Camera capture resolution.')
    parser.add_argument('-f', '--flip',
                        type=int, default=-1,
                        help='Flip video by cv2.flip(data, N).')
    parser.add_argument('-i', '--bg-image', metavar='IMAGE_FILE',
                        type=str, default="bg.jpg",
                        help='Background image.')
    parser.add_argument('-e', '--bg-video', metavar='VIDEO_FILE',
                        type=str, default=None,
                        help='Background video.')
    parser.add_argument('-u', '--bg-url', metavar='VIDEO_URL',
                        type=str, default=None,
                        help='Background video url.')
    parser.add_argument('-g', '--show-only-gui',
                        action='store_true',
                        help='Show results visually (needs X11).')

    ARGS = parser.parse_args()
    main(ARGS)
