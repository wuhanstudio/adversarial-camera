#ifndef __V4L2_DEV_H__
#define __V4L2_DEV_H__

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

/* Enable debug prints. */
#define ENABLE_BUFFER_DEBUG

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define pixfmtstr(x) (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, ((x) >> 24) & 0xff

/* ---------------------------------------------------------------------------
 * Generic stuff
 */

/* IO methods supported */
enum io_method {
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

/* Buffer representing one video frame */
struct buffer {
    struct v4l2_buffer buf;
    void *start;
    size_t length;
};

/* ---------------------------------------------------------------------------
 * V4L2 and UVC device instances
 */

/* Represents a V4L2 based video capture device */
struct v4l2_device {
    /* v4l2 device specific */
    int v4l2_fd;
    int is_streaming;
    char *v4l2_devname;

    /* v4l2 buffer specific */
    enum io_method io;
    struct buffer *mem;
    unsigned int nbufs;

    /* v4l2 buffer queue and dequeue counters */
    unsigned long long int qbuf_count;
    unsigned long long int dqbuf_count;

    /* uvc device hook */
    struct uvc_device *udev;
};

/* ---------------------------------------------------------------------------
 * V4L2 streaming related
 */

static int v4l2_uninit_device(struct v4l2_device *dev)
{
    unsigned int i;
    int ret;

    switch (dev->io) {
    case IO_METHOD_MMAP:
        for (i = 0; i < dev->nbufs; ++i) {
            ret = munmap(dev->mem[i].start, dev->mem[i].length);
            if (ret < 0) {
                printf("V4L2: munmap failed\n");
                return ret;
            }
        }

        free(dev->mem);
        break;

    case IO_METHOD_USERPTR:
    default:
        break;
    }

    return 0;
}

static int v4l2_reqbufs_mmap(struct v4l2_device *dev, int nbufs)
{
    struct v4l2_requestbuffers req;
    unsigned int i = 0;
    int ret;

    CLEAR(req);

    req.count = nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(dev->v4l2_fd, VIDIOC_REQBUFS, &req);
    if (ret < 0) {
        if (ret == -EINVAL)
            printf("V4L2: does not support memory mapping\n");
        else
            printf("V4L2: VIDIOC_REQBUFS error %s (%d).\n", strerror(errno), errno);
        goto err;
    }

    if (!req.count)
        return 0;

    if (req.count < 2) {
        printf("V4L2: Insufficient buffer memory.\n");
        ret = -EINVAL;
        goto err;
    }

    /* Map the buffers. */
    dev->mem = (buffer*) calloc(req.count, sizeof dev->mem[0]);
    if (!dev->mem) {
        printf("V4L2: Out of memory\n");
        ret = -ENOMEM;
        goto err;
    }

    for (i = 0; i < req.count; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->v4l2_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            printf(
                "V4L2: VIDIOC_QUERYBUF failed for buf %d: "
                "%s (%d).\n",
                i, strerror(errno), errno);
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].start =
            mmap(NULL /* start anywhere */, dev->mem[i].buf.length, PROT_READ | PROT_WRITE /* required */,
                 MAP_SHARED /* recommended */, dev->v4l2_fd, dev->mem[i].buf.m.offset);

        if (MAP_FAILED == dev->mem[i].start) {
            printf("V4L2: Unable to map buffer %u: %s (%d).\n", i, strerror(errno), errno);
            dev->mem[i].length = 0;
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].length = dev->mem[i].buf.length;
        printf("V4L2: Buffer %u mapped at address %p, length %d.\n", i, dev->mem[i].start, dev->mem[i].length);
    }

    dev->nbufs = req.count;
    printf("V4L2: %u buffers allocated.\n", req.count);

    return 0;

err_free:
    free(dev->mem);
err:
    return ret;
}

static int v4l2_reqbufs_userptr(struct v4l2_device *dev, int nbufs)
{
    struct v4l2_requestbuffers req;
    int ret;

    CLEAR(req);

    req.count = nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->v4l2_fd, VIDIOC_REQBUFS, &req);
    if (ret < 0) {
        if (ret == -EINVAL)
            printf("V4L2: does not support user pointer i/o\n");
        else
            printf("V4L2: VIDIOC_REQBUFS error %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    dev->nbufs = req.count;
    printf("V4L2: %u buffers allocated.\n", req.count);

    return 0;
}

static int v4l2_reqbufs(struct v4l2_device *dev, int nbufs)
{
    int ret = 0;

    switch (dev->io) {
    case IO_METHOD_MMAP:
        ret = v4l2_reqbufs_mmap(dev, nbufs);
        break;

    case IO_METHOD_USERPTR:
        ret = v4l2_reqbufs_userptr(dev, nbufs);
        break;

    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

static int v4l2_qbuf_mmap(struct v4l2_device *dev)
{
    unsigned int i;
    int ret;

    for (i = 0; i < dev->nbufs; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->v4l2_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            printf("V4L2: VIDIOC_QBUF failed : %s (%d).\n", strerror(errno), errno);
            return ret;
        }

        dev->qbuf_count++;
    }

    return 0;
}

static int v4l2_qbuf(struct v4l2_device *dev)
{
    int ret = 0;

    switch (dev->io) {
    case IO_METHOD_MMAP:
        ret = v4l2_qbuf_mmap(dev);
        break;

    case IO_METHOD_USERPTR:
        /* Empty. */
        ret = 0;
        break;

    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

/* ---------------------------------------------------------------------------
 * V4L2 generic stuff
 */

static int v4l2_get_format(struct v4l2_device *dev)
{
    struct v4l2_format fmt;
    int ret;

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl(dev->v4l2_fd, VIDIOC_G_FMT, &fmt);
    if (ret < 0) {
        return ret;
    }

    printf("V4L2: Getting current format: %c%c%c%c %ux%u\n", pixfmtstr(fmt.fmt.pix.pixelformat), fmt.fmt.pix.width,
           fmt.fmt.pix.height);

    return 0;
}

static int v4l2_set_format(struct v4l2_device *dev, struct v4l2_format *fmt)
{
    int ret;

    ret = ioctl(dev->v4l2_fd, VIDIOC_S_FMT, fmt);
    if (ret < 0) {
        printf("V4L2: Unable to set format %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    printf("V4L2: Setting format to: %c%c%c%c %ux%u\n", pixfmtstr(fmt->fmt.pix.pixelformat), fmt->fmt.pix.width,
           fmt->fmt.pix.height);

    return 0;
}

static int v4l2_set_ctrl(struct v4l2_device *dev, int new_val, int ctrl)
{
    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;
    int ret;

    CLEAR(queryctrl);

    switch (ctrl) {
    case V4L2_CID_BRIGHTNESS:
        queryctrl.id = V4L2_CID_BRIGHTNESS;
        ret = ioctl(dev->v4l2_fd, VIDIOC_QUERYCTRL, &queryctrl);
        if (-1 == ret) {
            if (errno != EINVAL)
                printf(
                    "V4L2: VIDIOC_QUERYCTRL"
                    " failed: %s (%d).\n",
                    strerror(errno), errno);
            else
                printf(
                    "V4L2_CID_BRIGHTNESS is not"
                    " supported: %s (%d).\n",
                    strerror(errno), errno);

            return ret;
        } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
            printf("V4L2_CID_BRIGHTNESS is not supported.\n");
            ret = -EINVAL;
            return ret;
        } else {
            CLEAR(control);
            control.id = V4L2_CID_BRIGHTNESS;
            control.value = new_val;

            ret = ioctl(dev->v4l2_fd, VIDIOC_S_CTRL, &control);
            if (-1 == ret) {
                printf("V4L2: VIDIOC_S_CTRL failed: %s (%d).\n", strerror(errno), errno);
                return ret;
            }
        }
        printf("V4L2: Brightness control changed to value = 0x%x\n", new_val);
        break;

    default:
        /* TODO: We don't support any other controls. */
        return -EINVAL;
    }

    return 0;
}

static int v4l2_start_capturing(struct v4l2_device *dev)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret;

    ret = ioctl(dev->v4l2_fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
        printf("V4L2: Unable to start streaming: %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    // printf("V4L2: Starting video stream.\n");

    return 0;
}

static int v4l2_stop_capturing(struct v4l2_device *dev)
{
    enum v4l2_buf_type type;
    int ret;

    switch (dev->io) {
    case IO_METHOD_MMAP:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ret = ioctl(dev->v4l2_fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0) {
            printf("V4L2: VIDIOC_STREAMOFF failed: %s (%d).\n", strerror(errno), errno);
            return ret;
        }

        break;
    default:
        /* Nothing to do. */
        break;
    }

    return 0;
}

static int v4l2_open(struct v4l2_device **v4l2, char *devname, struct v4l2_format *s_fmt)
{
    struct v4l2_device *dev;
    struct v4l2_capability cap;
    int fd;
    int ret = -EINVAL;

    fd = open(devname, O_RDWR | O_NONBLOCK, 0);
    if (fd == -1) {
        printf("V4L2: device open failed: %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        printf("V4L2: VIDIOC_QUERYCAP failed: %s (%d).\n", strerror(errno), errno);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        printf("V4L2: %s is no video capture device\n", devname);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        printf("V4L2: %s does not support streaming i/o\n", devname);
        goto err;
    }

    dev = (v4l2_device*) calloc(1, sizeof *dev);
    if (dev == NULL) {
        ret = -ENOMEM;
        goto err;
    }

    printf("V4L2 device is %s on bus %s\n", cap.card, cap.bus_info);

    dev->v4l2_fd = fd;

    /* Get the default image format supported. */
    ret = v4l2_get_format(dev);
    if (ret < 0)
        goto err_free;

    /*
     * Set the desired image format.
     * Note: VIDIOC_S_FMT may change width and height.
     */
    ret = v4l2_set_format(dev, s_fmt);
    if (ret < 0)
        goto err_free;

    /* Get the changed image format. */
    ret = v4l2_get_format(dev);
    if (ret < 0)
        goto err_free;

    printf("v4l2 open succeeded, file descriptor = %d\n", fd);

    *v4l2 = dev;

    return 0;

err_free:
    free(dev);
err:
    close(fd);

    return ret;
}

static void v4l2_close(struct v4l2_device *dev)
{
    close(dev->v4l2_fd);
    free(dev);
}

#endif // __V4L2_DEV_H__
