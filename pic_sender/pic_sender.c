#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

/*#define WIDTH 1920
#define HEIGHT 1080*/

#define WIDTH 2880
#define HEIGHT 1624

#define VIDEO_DEVICE1 "/dev/video0"
#define VIDEO_DEVICE2 "/dev/video2"
#define VIDEO_DEVICE3 "/dev/video4"
#define OUTPUT_FILE1 "image1.jpg"
#define OUTPUT_FILE2 "image2.jpg"
#define OUTPUT_FILE3 "image3.jpg"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
// #define SERVER_ADDRESS "172.30.1.98"
// #define SERVER_ADDRESS "192.168.0.51"
// #define SERVER_ADDRESS "192.168.0.3"
#define SERVER_ADDRESS "59.11.52.50"
#define BUFFER_SIZE 1024

int client_jpg_sender(char *jpgname)
{
    int client_fd;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    int file_fd;

    // Create a socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // Open file to send
    if ((file_fd = open(jpgname, O_RDONLY)) == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Send file data to server
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0)
    {
        if (send(client_fd, buffer, bytes_read, 0) == -1)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("File sent successfully\n");

    // Close sockets and file descriptor
    close(client_fd);
    close(file_fd);

    return 0;
}

int take_picture(char *cam_name, char *jpgname)
{
    int fd;
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    void *buffer;
    FILE *fp;

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // Open the video device
    fd = open(cam_name, O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open video device");
        return EXIT_FAILURE;
    }

    // Get device capabilities
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
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
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        perror("Failed to set video format");
        close(fd);
        return EXIT_FAILURE;
    }

    // Request buffers
    memset(&req, 0, sizeof(req));
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    req.count = 1;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        perror("Failed to request buffers");
        close(fd);
        return EXIT_FAILURE;
    }

    // Map the buffers
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
    {
        perror("Failed to query buffer");
        close(fd);
        return EXIT_FAILURE;
    }
    buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    if (buffer == MAP_FAILED)
    {
        perror("Failed to map buffer");
        close(fd);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 20; ++i) // Frame skip
    {
        // Queue buffer
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
        {
            perror("Failed to queue buffer");
            close(fd);
            return EXIT_FAILURE;
        }

        // Start capture
        if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
        {
            perror("Failed to start capture");
            close(fd);
            return EXIT_FAILURE;
        }

        // Dequeue buffer
        if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
        {
            perror("Failed to dequeue buffer");
            close(fd);
            return EXIT_FAILURE;
        }
    }

    // Stop capture
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
    {
        perror("Failed to stop capture");
        close(fd);
        return EXIT_FAILURE;
    }

    // Save captured image as a JPEG file
    fp = fopen(jpgname, "wb");
    if (fp == NULL)
    {
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

    printf("Image captured and saved as %s\n", jpgname);
}

int main()
{
    take_picture(VIDEO_DEVICE1, OUTPUT_FILE1);
    client_jpg_sender(OUTPUT_FILE1);

    take_picture(VIDEO_DEVICE2, OUTPUT_FILE2);
    client_jpg_sender(OUTPUT_FILE2);

    take_picture(VIDEO_DEVICE3, OUTPUT_FILE3);
    client_jpg_sender(OUTPUT_FILE3);

    return EXIT_SUCCESS;
}
