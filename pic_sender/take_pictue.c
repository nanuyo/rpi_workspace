#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define WIDTH 1920
#define HEIGHT 1080
#define VIDEO_DEVICE "/dev/video0"
#define OUTPUT_FILE "captured_image.jpg"

int main() {
    int fd;
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    void* buffer;
    FILE* fp;

    // Open the video device
    fd = open(VIDEO_DEVICE, O_RDWR);
    if (fd == -1) {
        perror("Failed to open video device");
        return EXIT_FAILURE;
    }

    // Get device capabilities
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("Failed to get device capabilities");
        close(fd);
        return EXIT_FAILURE;
    }

    // Set video format
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Failed to set video format");
        close(fd);
        return EXIT_FAILURE;
    }

    // Request buffers
    memset(&req, 0, sizeof(req));
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    req.count = 1;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Failed to request buffers");
        close(fd);
        return EXIT_FAILURE;
    }

    // Map the buffers
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
        perror("Failed to query buffer");
        close(fd);
        return EXIT_FAILURE;
    }
    buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    if (buffer == MAP_FAILED) {
        perror("Failed to map buffer");
        close(fd);
        return EXIT_FAILURE;
    }

    // Queue buffer
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Failed to queue buffer");
        close(fd);
        return EXIT_FAILURE;
    }

    // Start capture
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Failed to start capture");
        close(fd);
        return EXIT_FAILURE;
    }

    // Dequeue buffer
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Failed to dequeue buffer");
        close(fd);
        return EXIT_FAILURE;
    }

    // Stop capture
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        perror("Failed to stop capture");
        close(fd);
        return EXIT_FAILURE;
    }

    // Save captured image as a JPEG file
    fp = fopen(OUTPUT_FILE, "wb");
    if (fp == NULL) {
        perror("Failed to open output file");
        close(fd);
        return EXIT_FAILURE;
    }
    fwrite(buffer, 1, buf.bytesused, fp);
    fclose(fp);

    // Unmap buffer
    munmap(buffer, buf.length);

    // Close video device
    close(fd);

    printf("Image captured and saved as %s\n", OUTPUT_FILE);

    return EXIT_SUCCESS;
}
