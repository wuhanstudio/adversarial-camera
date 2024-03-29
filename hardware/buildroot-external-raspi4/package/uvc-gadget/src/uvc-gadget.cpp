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

#include "npy.hpp"
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

#define IMAGE_WIDTH_720P    1280
#define IMAGE_HEIGHT_720P   720

#include <fstream>
int recording = 0;
int capturing = 0;
cv::VideoWriter writer;

double clockToMilliseconds(clock_t ticks)
{
    // units/(units/time) => time (seconds) * 1000 = milliseconds
    return (ticks / (double)CLOCKS_PER_SEC) * 1000.0;
}

clock_t deltaTime = 0;
unsigned int frames = 0;
double frameRate = 30;
double averageFrameTimeMilliseconds = 33.333;

/* Frame format/resolution related params. */
int default_format = 0;         /* V4L2_PIX_FMT_YUYV */
int default_resolution = 0;     /* VGA 360p */

// Numpy noises
int add_noise = 0;
int *d;
std::vector<unsigned long> shape;
bool fortran_order;

static int v4l2_process_data(struct v4l2_device *dev)
{
    int ret;
    cv::Mat origin_img;
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
    switch (dev->io)
    {
    case IO_METHOD_USERPTR:
        vbuf.memory = V4L2_MEMORY_USERPTR;
        break;

    case IO_METHOD_MMAP:
    default:
        vbuf.memory = V4L2_MEMORY_MMAP;
        break;
    }

    ret = ioctl(dev->v4l2_fd, VIDIOC_DQBUF, &vbuf);
    if (ret < 0)
    {
        return ret;
    }

    dev->dqbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
    printf("Dequeueing buffer at V4L2 side = %d\n", vbuf.index);
#endif

    int width = (default_resolution == 0) ? IMAGE_WIDTH_360P : IMAGE_WIDTH_720P;
    int height = (default_resolution == 0) ? IMAGE_HEIGHT_360P : IMAGE_HEIGHT_720P;

    std::fstream fnoise;
    fnoise.open("noise");

    if (fnoise.is_open())
    {
        std::vector<double> data;
        npy::LoadArrayFromNumpy("noise.npy", shape, fortran_order, data);
        std::cout << "noise shape: ";
        for (size_t i = 0; i < shape.size(); i++)
            std::cout << shape[i] << ", ";
        std::cout << std::endl;

        if (d != NULL)
        {
            free(d);
        }
        d = (int *)malloc(sizeof(int) * data.size());
        for (size_t i = 0; i < data.size(); i++)
            d[i] = int(data[i] * 255);
        fnoise.close();
        remove("noise");
    }

    std::fstream fin;
    fin.open("capture");

    if (fin.is_open())
    {
        capturing = 1;
        fin.close();
    }

    std::fstream fstart;
    fstart.open("start");

    if (fstart.is_open())
    {
        recording = 1;
        fstart.close();
        remove("start");
        int codec = cv::VideoWriter::fourcc('M', 'P', '4', 'V');
        double fps = 30.0;
        std::string filename = "live.mp4";
        cv::Size sizeFrame(width, height);
        writer.open(filename, codec, fps, sizeFrame, 1);
        std::cout << "Started writing video... " << std::endl;
    }

    std::fstream fstop;
    fstop.open("stop");

    if (fstop.is_open())
    {
        recording = 0;
        fstop.close();
        remove("stop");
        std::cout << "Write complete !" << std::endl;
        writer.release();
    }

    // MJPEG --> MJPEG
    if (default_format == 1)
    {
        // Decode JPEG
        cv::_InputArray pic_arr((uint8_t *)dev->mem[vbuf.index].start, width * height * 3);
        cv::Mat out_img = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

        if (recording || capturing)
        {
            origin_img = out_img.clone();
        }

        if (out_img.data == NULL)
        {
            printf("Error decoding");
        }

        // You may apply OpenCV image processing here
        // Begin OpenCV
        // ...........
        // cv::cvtColor(out_img, out_img, cv::COLOR_BGR2RGB);

        if (add_noise)
        {
            clock_t beginFrame = clock();

            cv::resize(out_img, out_img, cv::Size(shape[1], shape[0]), cv::INTER_LINEAR);

            for (int i = 0; i < shape[0]; i++)
            {
                for (int j = 0; j < shape[1]; j++)
                {
                    // get pixel
                    cv::Vec3b &color = out_img.at<cv::Vec3b>(i, j);
                    int *noise_c = &d[i * out_img.cols * 3 + j * 3];
                    uint8_t temp;

                    temp = color[0];
                    color[0] += noise_c[2];
                    if ((noise_c[2] < 0) && (color[0] > temp))
                        color[0] = 0;
                    if ((noise_c[2] > 0) && (color[0] < temp))
                        color[0] = 255;

                    temp = color[1];
                    color[1] += noise_c[1];
                    if ((noise_c[1] < 0) && (color[1] > temp))
                        color[1] = 0;
                    if ((noise_c[1] > 0) && (color[1] < temp))
                        color[1] = 255;

                    temp = color[2];
                    color[2] += noise_c[0];
                    if ((noise_c[0] < 0) && (color[2] > temp))
                        color[2] = 0;
                    if ((noise_c[0] > 0) && (color[2] < temp))
                        color[2] = 255;
                }
            }

            /*
            for(int i = 0; i < out_img.rows; i++) {
                    for(int j = 0; j < out_img.cols; j++) {
                        // get pixel
                        cv::Vec3b& color = out_img.at<cv::Vec3b>(i, j);
                        int* color1 = &d[i * out_img.cols * 3 + j * 3 ];

                        for (size_t k = 0; k < 3; k++)
                        {
                            if (color1[k] < 0) {
                                if (int(color[2-k]) <= int((-color1[k])))
                                    color[2-k] = 0;
                                else
                                    color[2-k] += color1[k];
                            }
                            else {
                                if ( (255 - color[2-k]) <= color1[k])
                                    color[2-k] = 255;
                                else
                                    color[2-k] += color1[k];
                            }
                        }
                    }
                }
            */

            clock_t endFrame = clock();

            deltaTime += endFrame - beginFrame;
            frames++;

            // if you really want FPS
            if (clockToMilliseconds(deltaTime) > 1000.0)
            {
                // every second
                frameRate = (double)frames * 0.5 + frameRate * 0.5; // more stable
                frames = 0;
                deltaTime -= CLOCKS_PER_SEC;
                averageFrameTimeMilliseconds = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);

                std::cout << "CPU time was:" << averageFrameTimeMilliseconds << std::endl;
            }

            cv::resize(out_img, out_img, cv::Size(width, height), cv::INTER_LINEAR);
        }

        // cv::cvtColor(out_img, out_img, cv::COLOR_RGB2BGR);
        // End   OpenCV

        // Encode JPEG
        std::vector<uchar> outbuffer;
        cv::imencode(".jpg", out_img, outbuffer);

        uint32_t outlen = sizeof(uchar) * outbuffer.size();
        vbuf.length = outlen;
        vbuf.bytesused = outlen;
        memcpy((uint8_t *)dev->mem[vbuf.index].start, outbuffer.data(), outlen);
    }
    // YUYV --> MJPEG
    else
    {
        // YUYV to JPEG
        // uint8_t* outbuffer = NULL;
        // cv::Mat input = img.reshape(1, img.total()*img.channels());
        // std::vector<uint8_t> vec = img.isContinuous()? input : input.clone();
        // uint32_t outlen = compressYUYVtoJPEG(vec.data(), 640, 360, outbuffer);

        // YUYV to RGB
        cv::Mat out_img = cv::Mat(cv::Size(width, height), CV_8UC2, dev->mem[vbuf.index].start);

        // cv::Mat out_img = cv::Mat(cv::Size(width, height), CV_8UC2, dev->mem[vbuf.index].start);
        if (recording || capturing)
        {
            origin_img = out_img.clone();
        }

        cv::cvtColor(out_img, out_img, cv::COLOR_YUV2RGB_YVYU);

        // You may apply OpenCV image processing here
        // Begin OpenCV
        // ...........
        if (add_noise)
        {
            cv::resize(out_img, out_img, cv::Size(shape[1], shape[0]), cv::INTER_LINEAR);

            for (int i = 0; i < shape[0]; i++)
            {
                for (int j = 0; j < shape[1]; j++)
                {
                    // get pixel
                    cv::Vec3b &color = out_img.at<cv::Vec3b>(i, j);
                    int *color1 = &d[i * out_img.cols * 3 + j * 3];
                    uint8_t temp;

                    temp = color[0];
                    color[0] += color1[2];
                    if ((color1[2] < 0) && (color[0] > temp))
                        color[0] = 0;
                    if ((color1[2] > 0) && (color[0] < temp))
                        color[0] = 255;

                    temp = color[1];
                    color[1] += color1[1];
                    if ((color1[1] < 0) && (color[1] > temp))
                        color[1] = 0;
                    if ((color1[1] > 0) && (color[1] < temp))
                        color[1] = 255;

                    temp = color[2];
                    color[2] += color1[0];
                    if ((color1[0] < 0) && (color[2] > temp))
                        color[2] = 0;
                    if ((color1[0] > 0) && (color[2] < temp))
                        color[2] = 255;
                }
            }

            cv::resize(out_img, out_img, cv::Size(width, height), cv::INTER_LINEAR);
        }
        // End   OpenCV

        // RGB to YV12
        cv::cvtColor(out_img, out_img, cv::COLOR_RGB2YUV_YV12);

        // YV12 to JPEG
        uint8_t *outbuffer = NULL;
        cv::Mat input = out_img.reshape(1, out_img.total() * out_img.channels());
        std::vector<uint8_t> vec = out_img.isContinuous() ? input : input.clone();
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

    if (capturing)
    {
        imwrite("input.jpg", origin_img);
        capturing = 0;
        remove("capture");
    }

    if (recording)
    {
        cv::Size sizeFrame(width, height);
        cv::Mat xframe;
        cv::resize(origin_img, xframe, sizeFrame);
        writer.write(xframe);
    }

    // cv::imshow("origin", origin_img);
    // cv::waitKey(1);

    /* Queue video buffer to UVC domain. */
    CLEAR(ubuf);

    ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    switch (dev->udev->io)
    {
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
    if (ret < 0)
    {
        /* Check for a USB disconnect/shutdown event. */
        if (errno == ENODEV)
        {
            dev->udev->uvc_shutdown_requested = 1;
            printf(
                "UVC: Possible USB shutdown requested from "
                "Host, seen during VIDIOC_QBUF\n");
            return 0;
        }
        else
        {
            return ret;
        }
    }

    dev->udev->qbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
    printf("Queueing buffer at UVC side = %d\n", ubuf.index);
#endif

    if (!dev->udev->first_buffer_queued && !dev->udev->run_standalone)
    {
        uvc_video_stream(dev->udev, 1);
        dev->udev->first_buffer_queued = 1;
        dev->udev->is_streaming = 1;
    }

    return 0;
}

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s [options]\n", argv0);
    fprintf(stderr, "Available options are\n");
    fprintf(stderr,
            " -f <format> Select frame format\n\t"
            "0 = V4L2_PIX_FMT_YUYV\n\t"
            "1 = V4L2_PIX_FMT_MJPEG\n");
    fprintf(stderr,
            " -r <resolution> Select frame resolution:\n\t"
            "0 = 360p, VGA (640x360)\n\t"
            "1 = 720p, HD (1280x720)\n");
    fprintf(stderr, " -u device    UVC Video Output device\n");
    fprintf(stderr, " -v device    V4L2 Video Capture device\n");
    fprintf(stderr, " -n noise    Add adversarial perturbation in noise.npy\n");
    fprintf(stderr, " -h help    Print this help screen and exit\n");
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

    /* Ping-Pong buffers */
    int nbufs = 2;

    /* USB speed related params */
    int mult = 0;
    int burst = 0;
    enum usb_device_speed speed = USB_SPEED_SUPER; /* High-Speed */

    enum io_method uvc_io_method = IO_METHOD_USERPTR;

    while ((opt = getopt(argc, argv, "f:r:u:v:nh")) != -1)
    {
        switch (opt)
        {
        case 'f':
            if (atoi(optarg) < 0 || atoi(optarg) > 1)
            {
                usage(argv[0]);
                return 1;
            }

            default_format = atoi(optarg);
            break;

        case 'r':
            if (atoi(optarg) < 0 || atoi(optarg) > 1)
            {
                usage(argv[0]);
                return 1;
            }

            default_resolution = atoi(optarg);
            break;

        case 'u':
            uvc_devname = optarg;
            break;

        case 'v':
            v4l2_devname = optarg;
            break;

        case 'n':
            add_noise = 1;
            break;

        case 'h':
            usage(argv[0]);
            return 1;

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

    /* Bind UVC and V4L2 devices. */
    udev->uvc_devname = uvc_devname;
    vdev->v4l2_devname = v4l2_devname;
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
    switch (uvc_io_method)
    {
    case IO_METHOD_MMAP:
        vdev->io = IO_METHOD_USERPTR;
        break;

    case IO_METHOD_USERPTR:
    default:
        vdev->io = IO_METHOD_MMAP;
        break;
    }

    udev->maxpkt = 1024;

    if (IO_METHOD_MMAP == vdev->io)
    {
        /*
         * Ensure that the V4L2 video capture device has already some
         * buffers queued.
         */
        v4l2_reqbufs(vdev, vdev->nbufs);
    }

    /* Init UVC events. */
    uvc_events_init(udev);

    if (add_noise)
    {
        std::vector<double> data;
        npy::LoadArrayFromNumpy("noise.npy", shape, fortran_order, data);
        std::cout << "noise shape: ";
        for (size_t i = 0; i < shape.size(); i++)
            std::cout << shape[i] << ", ";
        std::cout << std::endl;

        d = (int *)malloc(sizeof(int) * data.size());
        for (size_t i = 0; i < data.size(); i++)
            d[i] = int(data[i] * 255);
    }

    int width = (default_resolution == 0) ? IMAGE_WIDTH_360P : IMAGE_WIDTH_720P;
    int height = (default_resolution == 0) ? IMAGE_HEIGHT_360P : IMAGE_HEIGHT_720P;

    while (1)
    {
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

        if (-1 == ret)
        {
            printf("select error %d, %s\n", errno, strerror(errno));
            if (EINTR == errno)
                continue;

            break;
        }

        if (0 == ret)
        {
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

    if (vdev->is_streaming)
    {
        /* Stop V4L2 streaming... */
        v4l2_stop_capturing(vdev);
        v4l2_uninit_device(vdev);
        v4l2_reqbufs(vdev, 0);
        vdev->is_streaming = 0;
    }

    if (udev->is_streaming)
    {
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
