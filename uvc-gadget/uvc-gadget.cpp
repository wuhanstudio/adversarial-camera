/*
 * UVC gadget test application
 *
 * Copyright (C) 2010 Ideas on board SPRL <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */

#include <stdio.h>
#include <string.h>

#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "uvc.h"

#include "uvc_dev.h"
#include "v4l2_dev.h"

#include "yuyv_to_jpeg.h"
#include "yv12_to_jpeg.h"

#define IMAGE_WIDTH_360P    640
#define IMAGE_HEIGHT_360P   360

#define IMAGE_WIDTH_720P	1280
#define IMAGE_HEIGHT_720P   720

/* Frame format/resolution related params. */
int default_format = 0;         /* V4L2_PIX_FMT_YUYV */
int default_resolution = 0;     /* VGA 360p */

static int v4l2_process_data(struct v4l2_device *dev)
{
    int ret;
    struct v4l2_buffer vbuf;
    struct v4l2_buffer ubuf;

    /* Return immediately if V4l2 streaming has not yet started. */
    if (!dev->is_streaming)
        return 0;

    if (dev->udev->first_buffer_queued)
        if (dev->dqbuf_count >= dev->qbuf_count)
            return 0;

    /* Dequeue spent buffer rom V4L2 domain. */
    CLEAR(vbuf);

    vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    switch (dev->io) {
    case IO_METHOD_USERPTR:
        vbuf.memory = V4L2_MEMORY_USERPTR;
        break;

    case IO_METHOD_MMAP:
    default:
        vbuf.memory = V4L2_MEMORY_MMAP;
        break;
    }

    ret = ioctl(dev->v4l2_fd, VIDIOC_DQBUF, &vbuf);
    if (ret < 0) {
        return ret;
    }

    dev->dqbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
    printf("Dequeueing buffer at V4L2 side = %d\n", vbuf.index);
#endif

    int width = (default_resolution == 0) ? IMAGE_WIDTH_360P : IMAGE_WIDTH_720P;
    int height = (default_resolution == 0) ? IMAGE_HEIGHT_360P : IMAGE_HEIGHT_720P;

    // MJPEG --> MJPEG
    if(default_format == 1) {
        // Decode JPEG
        cv::Mat out_img = cv::imdecode(cv::Mat(cv::Size(width, height), CV_8UC1, dev->mem[vbuf.index].start), cv::IMREAD_COLOR);
        if ( out_img.data == NULL )   
        {
            printf("Error decoding");
        }

        // Encode JPEG
        std::vector<uchar> outbuffer;
        cv::imencode(".jpg", out_img, outbuffer);

        uint32_t outlen = sizeof(uchar) * outbuffer.size();
        vbuf.length = outlen;
        vbuf.bytesused = outlen;
        memcpy(dev->mem[vbuf.index].start, outbuffer.data(), outlen);
    }
    // YUYV --> MJPEG
    else {
        // YUYV to JPEG
        // uint8_t* outbuffer = NULL;
        // cv::Mat input = img.reshape(1, img.total()*img.channels());
        // std::vector<uint8_t> vec = img.isContinuous()? input : input.clone();
        // uint32_t outlen = compressYUYVtoJPEG(vec.data(), 640, 360, outbuffer);

        // YUYV to RGB
        cv::Mat img = cv::Mat(cv::Size(width, height), CV_8UC2, dev->mem[vbuf.index].start);
        cv::Mat out_img;
        cv::cvtColor(img, out_img, cv::COLOR_YUV2RGB_YVYU);

        // RGB to YV12
        cv::cvtColor(out_img, img, cv::COLOR_RGB2YUV_YV12);

        // YV12 to JPEG
        uint8_t* outbuffer = NULL;
        cv::Mat input = img.reshape(1, img.total()*img.channels());
        std::vector<uint8_t> vec = img.isContinuous()? input : input.clone();
        uint32_t outlen = yv12_to_jpeg(vec.data(), width, height, outbuffer);

        // Copy to UVC device
        // dev->mem[vbuf.index].length = outlen; THIS CANNOT BE SET, CAUSES ERROR

        vbuf.length = outlen;
        vbuf.bytesused = outlen;
        memcpy(dev->mem[vbuf.index].start, outbuffer, outlen);

        // Write JPEG to file 
        // std::vector<uint8_t> output = std::vector<uint8_t>(outbuffer, outbuffer + outlen);
        // std::ofstream ofs("output.jpg", std::ios_base::binary);
        // ofs.write((const char*) &output[0], output.size());
        // ofs.close();
    }

    /* Queue video buffer to UVC domain. */
    CLEAR(ubuf);

    ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    switch (dev->udev->io) {
    case IO_METHOD_MMAP:
        ubuf.memory = V4L2_MEMORY_MMAP;
        ubuf.length = vbuf.length;
        ubuf.index = vbuf.index;
        ubuf.bytesused = vbuf.bytesused;
        break;

    case IO_METHOD_USERPTR:
    default:
        ubuf.memory = V4L2_MEMORY_USERPTR;
        ubuf.m.userptr = (unsigned long)dev->mem[vbuf.index].start;
        ubuf.length = dev->mem[vbuf.index].length;
        ubuf.index = vbuf.index;
        ubuf.bytesused = vbuf.bytesused;
        break;
    }

    ret = ioctl(dev->udev->uvc_fd, VIDIOC_QBUF, &ubuf);
    if (ret < 0) {
        /* Check for a USB disconnect/shutdown event. */
        if (errno == ENODEV) {
            dev->udev->uvc_shutdown_requested = 1;
            printf(
                "UVC: Possible USB shutdown requested from "
                "Host, seen during VIDIOC_QBUF\n");
            return 0;
        } else {
            return ret;
        }
    }

    dev->udev->qbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
    printf("Queueing buffer at UVC side = %d\n", ubuf.index);
#endif

    if (!dev->udev->first_buffer_queued && !dev->udev->run_standalone) {
        uvc_video_stream(dev->udev, 1);
        dev->udev->first_buffer_queued = 1;
        dev->udev->is_streaming = 1;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * main
 */

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s [options]\n", argv0);
    fprintf(stderr, "Available options are\n");
    fprintf(stderr,
            " -f <format>    Select frame format\n\t"
            "0 = V4L2_PIX_FMT_YUYV\n\t"
            "1 = V4L2_PIX_FMT_MJPEG\n");
    fprintf(stderr, " -h		Print this help screen and exit\n");
    fprintf(stderr, " -m		Streaming mult for ISOC (b/w 0 and 2)\n");
    fprintf(stderr, " -n		Number of Video buffers (b/w 2 and 32)\n");
    fprintf(stderr,
            " -o <IO method> Select UVC IO method:\n\t"
            "0 = MMAP\n\t"
            "1 = USER_PTR\n");
    fprintf(stderr,
            " -r <resolution> Select frame resolution:\n\t"
            "0 = 360p, VGA (640x360)\n\t"
            "1 = 720p, (1280x720)\n");
    fprintf(stderr,
            " -s <speed>	Select USB bus speed (b/w 0 and 2)\n\t"
            "0 = Full Speed (FS)\n\t"
            "1 = High Speed (HS)\n\t"
            "2 = Super Speed (SS)\n");
    fprintf(stderr, " -t		Streaming burst (b/w 0 and 15)\n");
    fprintf(stderr, " -u device	UVC Video Output device\n");
    fprintf(stderr, " -v device	V4L2 Video Capture device\n");
}

int main(int argc, char *argv[])
{
    struct uvc_device *udev;
    struct v4l2_device *vdev;
    struct timeval tv;
    struct v4l2_format fmt;
    char *uvc_devname = "/dev/video0";
    char *v4l2_devname = "/dev/video1";

    fd_set fdsv, fdsu;
    int ret, opt, nfds;

    int nbufs = 2;              /* Ping-Pong buffers */

    /* USB speed related params */
    int mult = 0;
    int burst = 0;
    enum usb_device_speed speed = USB_SPEED_SUPER; /* High-Speed */
    enum io_method uvc_io_method = IO_METHOD_USERPTR;

    while ((opt = getopt(argc, argv, "f:hm:n:o:r:s:t:u:v:")) != -1) {
        switch (opt) {
            case 'f':
                if (atoi(optarg) < 0 || atoi(optarg) > 1) {
                    usage(argv[0]);
                    return 1;
                }

                default_format = atoi(optarg);
                break;

            case 'h':
                usage(argv[0]);
                return 1;

            case 'm':
                if (atoi(optarg) < 0 || atoi(optarg) > 2) {
                    usage(argv[0]);
                    return 1;
                }

                mult = atoi(optarg);
                printf("Requested Mult value = %d\n", mult);
                break;

            case 'n':
                if (atoi(optarg) < 2 || atoi(optarg) > 32) {
                    usage(argv[0]);
                    return 1;
                }

                nbufs = atoi(optarg);
                printf("Number of buffers requested = %d\n", nbufs);
                break;

            case 'o':
                if (atoi(optarg) < 0 || atoi(optarg) > 1) {
                    usage(argv[0]);
                    return 1;
                }

                uvc_io_method = atoi(optarg);
                printf("UVC: IO method requested is %s\n", (uvc_io_method == IO_METHOD_MMAP) ? "MMAP" : "USER_PTR");
                break;

            case 'r':
                if (atoi(optarg) < 0 || atoi(optarg) > 1) {
                    usage(argv[0]);
                    return 1;
                }

                default_resolution = atoi(optarg);
                break;

            case 's':
                if (atoi(optarg) < 0 || atoi(optarg) > 2) {
                    usage(argv[0]);
                    return 1;
                }

                speed = atoi(optarg);
                break;

            case 't':
                if (atoi(optarg) < 0 || atoi(optarg) > 15) {
                    usage(argv[0]);
                    return 1;
                }

                burst = atoi(optarg);
                printf("Requested Burst value = %d\n", burst);
                break;

            case 'u':
                uvc_devname = optarg;
                break;

            case 'v':
                v4l2_devname = optarg;
                break;

            default:
                printf("Invalid option '-%c'\n", opt);
                usage(argv[0]);
                return 1;
        }
    }

    /*
     * Try to set the default format at the V4L2 video capture
     * device as requested by the user.
    */
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = (default_resolution == 0) ? IMAGE_WIDTH_360P : IMAGE_WIDTH_720P;
    fmt.fmt.pix.height = (default_resolution == 0) ? IMAGE_HEIGHT_360P : IMAGE_HEIGHT_720P;
    fmt.fmt.pix.sizeimage = (default_format == 0) ? (fmt.fmt.pix.width * fmt.fmt.pix.height * 2)
                                                    : (fmt.fmt.pix.width * fmt.fmt.pix.height * 1.5);
	printf("Format: %d 0:YUYV 1:MJPEG\n", default_format);
    fmt.fmt.pix.pixelformat = (default_format == 0) ? V4L2_PIX_FMT_YUYV : V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    /* Open the V4L2 device. */
    ret = v4l2_open(&vdev, v4l2_devname, &fmt);
    if (vdev == NULL || ret < 0)
        return 1;

    /* Open the UVC device. */
    ret = uvc_open(&udev, uvc_devname);
    if (udev == NULL || ret < 0)
        return 1;

    udev->uvc_devname = uvc_devname;

    vdev->v4l2_devname = v4l2_devname;
    /* Bind UVC and V4L2 devices. */
    udev->vdev = vdev;
    vdev->udev = udev;

    /* Set parameters as passed by user. */
    udev->width = (default_resolution == 0) ? IMAGE_WIDTH_360P : IMAGE_WIDTH_720P;
    udev->height = (default_resolution == 0) ? IMAGE_HEIGHT_360P : IMAGE_HEIGHT_720P;
    udev->imgsize = (default_format == 0) ? (udev->width * udev->height * 2) : (udev->width * udev->height * 1.5);
    udev->fcc = (default_format == 0) ? V4L2_PIX_FMT_YUYV : V4L2_PIX_FMT_MJPEG;
    udev->io = uvc_io_method;
    udev->bulk = 0;
    udev->nbufs = nbufs;
    udev->mult = mult;
    udev->burst = burst;
    udev->speed = speed;


    /* UVC - V4L2 integrated path */
    vdev->nbufs = nbufs;

    /*
        * IO methods used at UVC and V4L2 domains must be
        * complementary to avoid any memcpy from the CPU.
        */
    switch (uvc_io_method) {
    case IO_METHOD_MMAP:
        vdev->io = IO_METHOD_USERPTR;
        break;

    case IO_METHOD_USERPTR:
    default:
        vdev->io = IO_METHOD_MMAP;
        break;
    }

    switch (speed) {
    case USB_SPEED_FULL:
        /* Full Speed. */
        udev->maxpkt = 1023;

    case USB_SPEED_HIGH:
        /* High Speed. */
        udev->maxpkt = 1024;

    case USB_SPEED_SUPER:
    default:
        /* Super Speed. */
        udev->maxpkt = 1024;
    }

    if (IO_METHOD_MMAP == vdev->io) {
        /*
         * Ensure that the V4L2 video capture device has already some
         * buffers queued.
         */
        v4l2_reqbufs(vdev, vdev->nbufs);
    }

    /* Init UVC events. */
    uvc_events_init(udev);

    while (1) {
        FD_ZERO(&fdsv);
        FD_ZERO(&fdsu);

        /* We want both setup and data events on UVC interface.. */
        FD_SET(udev->uvc_fd, &fdsu);

        fd_set efds = fdsu;
        fd_set dfds = fdsu;

        /* ..but only data events on V4L2 interface */
        FD_SET(vdev->v4l2_fd, &fdsv);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        nfds = max(vdev->v4l2_fd, udev->uvc_fd);
        ret = select(nfds + 1, &fdsv, &dfds, &efds, &tv);

        if (-1 == ret) {
            printf("select error %d, %s\n", errno, strerror(errno));
            if (EINTR == errno)
                continue;

            break;
        }

        if (0 == ret) {
            printf("select timeout\n");
            break;
        }

        if (FD_ISSET(udev->uvc_fd, &efds))
            uvc_events_process(udev);
        if (FD_ISSET(udev->uvc_fd, &dfds))
            uvc_video_process(udev);
        if (FD_ISSET(vdev->v4l2_fd, &fdsv))
            v4l2_process_data(vdev);
    }

    if (vdev->is_streaming) {
        /* Stop V4L2 streaming... */
        v4l2_stop_capturing(vdev);
        v4l2_uninit_device(vdev);
        v4l2_reqbufs(vdev, 0);
        vdev->is_streaming = 0;
    }

    if (udev->is_streaming) {
        /* ... and now UVC streaming.. */
        uvc_video_stream(udev, 0);
        uvc_uninit_device(udev);
        uvc_video_reqbufs(udev, 0);
        udev->is_streaming = 0;
    }

    v4l2_close(vdev);

    uvc_close(udev);
    return 0;
}
