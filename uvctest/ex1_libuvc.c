#include "libuvc/libuvc.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/uvcvideo.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#define UVC_CTRL_NODE "/dev/video1"

int XU_Set_Cur(int fd, uint8_t xu_unit, uint8_t xu_selector, uint16_t xu_size, uint8_t *xu_data)
{
	int err=0;
	struct uvc_xu_control_query xctrlq;
	xctrlq.unit = xu_unit;
	xctrlq.selector = xu_selector;
	xctrlq.query = UVC_SET_CUR;
	xctrlq.size = xu_size;
	xctrlq.data = xu_data;
	if((err=ioctl(fd, UVCIOC_CTRL_QUERY, &xctrlq))<0)
		printf("\tUVCIOC_CTRL_QUERY - Error(%s)\n", strerror(errno));
	return err;
}

int XU_Get_Cur(int fd, uint8_t xu_unit, uint8_t xu_selector, uint16_t xu_size, uint8_t *xu_data)
{
	int err=0;
	struct uvc_xu_control_query xctrlq;
	xctrlq.unit = xu_unit;
	xctrlq.selector = xu_selector;
	xctrlq.query = UVC_GET_CUR;
	xctrlq.size = xu_size;
	xctrlq.data = xu_data;
	if((err=ioctl(fd, UVCIOC_CTRL_QUERY, &xctrlq))<0)
		printf("\tUVCIOC_CTRL_QUERY - Error(%s)\n", strerror(errno));
	return err;
}

int XU_Read(int fd, unsigned int addr, unsigned char *read_data)
{
	int ret = 0;
	uint8_t ctrldata[4];

	//uvc_xu_control parmeters
	uint8_t xu_unit= 3;
	uint8_t xu_selector= 0x01;
	uint16_t xu_size= 4;
	uint8_t *xu_data= ctrldata;

	xu_data[0] = (addr & 0xFF);
	xu_data[1] = ((addr >> 8) & 0xFF);
	xu_data[2] = 0x0;
	xu_data[3] = 0xFF;		/* Dummy Write */

	/* Dummy Write */
	if ((ret = XU_Set_Cur(fd, xu_unit, xu_selector, xu_size, xu_data)) < 0) {
		printf("XU_Set_Cur failed.(%d)\n", ret);
		return ret;
	}

	/* Status Read */
	xu_data[3] = 0x00;

	if ((ret = XU_Get_Cur(fd, xu_unit, xu_selector, xu_size, xu_data)) < 0) {
		printf("XU_Get_Cur failed.(%d)\n", ret);
		return ret;

	}
	*read_data = xu_data[2];

	return ret;
}

#define	XU_CAM_STATUS_ADDR	0x0311
// status_data[0] --> Camera preview (running): 0x01, No preview (No running) state: 0x00
int XU_Cam_Status_Read(int fd, unsigned char *status_data)
{
	int ret = 0;
    unsigned int status_addr = XU_CAM_STATUS_ADDR;
    unsigned char read_data = 0;

    if ((ret = XU_Read(fd, status_addr, &read_data)) < 0)
        return ret;

    *status_data = read_data;

	return ret;
}

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
int fd ;

void cb(uvc_frame_t *frame, void *ptr)
{
  uvc_frame_t *bgr;
  uvc_error_t ret;
  enum uvc_frame_format *frame_format = (enum uvc_frame_format *)ptr;
  FILE *fp;
  static int jpeg_count = 0;
  static const char *H264_FILE = "iOSDevLog.h264";
  static const char *MJPEG_FILE = ".jpeg";
  char filename[16];
   unsigned char status_data;

  
  /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
  bgr = uvc_allocate_frame(frame->width * frame->height * 3);
  if (!bgr)
  {
    printf("unable to allocate bgr frame!\n");
    return;
  }

  printf("callback! frame_format = %d, width = %d, height = %d, length = %lu, ptr = %d\n",
         frame->frame_format, frame->width, frame->height, frame->data_bytes, (int)ptr);

  switch (frame->frame_format)
  {
    //   case UVC_FRAME_FORMAT_H264:
    /* use `ffplay H264_FILE` to play */
    /* fp = fopen(H264_FILE, "a");
     * fwrite(frame->data, 1, frame->data_bytes, fp);
     * fclose(fp); */
    // break;
  case UVC_COLOR_FORMAT_MJPEG:

  //  ret = XU_Cam_Status_Read(fd, &status_data);
	// 		if (ret < 0) {
	// 			printf("XU_Cam_Status_Read failed (%d)\n", ret);
	// 		} else {
	// 			if( (status_data & 0x01) == 1) // Preview
	// 			{	
	// 				if( ((status_data & 0x02)>> 1) == 1 )
	// 					printf("AE = 1, ");
	// 				else 
	// 					printf("AE = 0, ");

	// 				if( ((status_data & 0x04)>> 2) == 1 )
	// 					printf("AWB = 1\n ");
	// 				else 
	// 					printf("AWB = 0\n");
	// 			}
	// 			else
	// 			 printf("No preview, ");
	// 		}
 
    sprintf(filename, "%d%s", jpeg_count++, MJPEG_FILE);
    fp = fopen(filename, "w");
    fwrite(frame->data, 1, frame->data_bytes, fp);
    fclose(fp);
    break;
  case UVC_COLOR_FORMAT_YUYV:
    /* Do the BGR conversion */
    ret = uvc_any2bgr(frame, bgr);
    if (ret)
    {
      uvc_perror(ret, "uvc_any2bgr");
      uvc_free_frame(bgr);
      return;
    }
    break;
  default:
    break;
  }

  if (frame->sequence % 30 == 0)
  {
    printf(" * got image %u\n", frame->sequence);
  }

  /* Call a user function:
   *
   * my_type *my_obj = (*my_type) ptr;
   * my_user_function(ptr, bgr);
   * my_other_function(ptr, bgr->data, bgr->width, bgr->height);
   */

  /* Call a C++ method:
   *
   * my_type *my_obj = (*my_type) ptr;
   * my_obj->my_func(bgr);
   */

  /* Use opencv.highgui to display the image:
   *
   * cvImg = cvCreateImageHeader(
   *     cvSize(bgr->width, bgr->height),
   *     IPL_DEPTH_8U,
   *     3);
   *
   * cvSetData(cvImg, bgr->data, bgr->width * 3);
   *
   * cvNamedWindow("Test", CV_WINDOW_AUTOSIZE);
   * cvShowImage("Test", cvImg);
   * cvWaitKey(10);
   *
   * cvReleaseImageHeader(&cvImg);
   */

  uvc_free_frame(bgr);
}



int main(int argc, char **argv)
{
  uvc_context_t *ctx;
  uvc_device_t *dev;
  uvc_device_handle_t *devh;
  uvc_stream_ctrl_t ctrl;
  uvc_error_t res;
 

  // fd = open(UVC_CTRL_NODE, O_RDWR);
  // if (fd < 0) {
  // 	printf("File open err. (%d)\n", fd);
  // 	return;
  // }

 

  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
  res = uvc_init(&ctx, NULL);

  if (res < 0)
  {
    uvc_perror(res, "uvc_init");
    return res;
  }

  puts("UVC initialized");

  /* Locates the first attached UVC device, stores in dev */
  res = uvc_find_device(
      ctx, &dev,
      0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */

  if (res < 0)
  {
    uvc_perror(res, "uvc_find_device"); /* no devices found */
    return res;
  }
  else
  {
    puts("Device found");
  }

  /* Try to open the device: requires exclusive access */
  res = uvc_open(dev, &devh);

  if (res < 0)
  {
    uvc_perror(res, "uvc_open"); /* unable to open device */
    puts("Please use sudo");
    return res;
  }
  else
  {
    puts("Device opened");
  }

  /* Print out a message containing all the information that libuvc
   * knows about the device */
  uvc_print_diag(devh, stderr);

  const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh);
  const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
  enum uvc_frame_format frame_format;
  int width = 640;
  int height = 480;
  int fps = 30;

  switch (format_desc->bDescriptorSubtype)
  {
  case UVC_VS_FORMAT_MJPEG:
    frame_format = UVC_FRAME_FORMAT_MJPEG;
    puts("UVC_FRAME_FORMAT_MJPEG");
    break;
  case UVC_VS_FORMAT_FRAME_BASED:
    frame_format = UVC_FRAME_FORMAT_MJPEG;
    puts("UVC_FRAME_FORMAT_MJPEG 2");
    break;
  default:
    frame_format = UVC_FRAME_FORMAT_YUYV;
    puts("UVC_FRAME_FORMAT_YUYV");
    break;
  }

  if (frame_desc)
  {
    width = frame_desc->wWidth;
    height = frame_desc->wHeight;
    fps = 10000000 / frame_desc->dwDefaultFrameInterval;
  }

  printf("\nFirst format: (%4s) %dx%d %dfps\n", format_desc->fourccFormat, width, height, fps);

  /* Try to negotiate first stream profile */
  res = uvc_get_stream_ctrl_format_size(
      devh, &ctrl, /* result stored in ctrl */
      frame_format,
      width, height, fps /* width, height, fps */
  );

  if (res < 0)
  {
    uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
    return res;
  }
  else
  {
    puts("Success uvc_get_stream_ctrl_format_size");
  }
  /* Print out the result */
  uvc_print_stream_ctrl(&ctrl, stderr);

  res = uvc_start_streaming(devh, &ctrl, cb, (void *)12345, 0);

  if (res < 0)
  {
    uvc_perror(res, "start_streaming"); /* unable to start stream */
    return res;
  }
  else
  {
    puts("Streaming...");
  }

  sleep(5); /* stream for 10 seconds */

  /* End the stream. Blocks until last callback is serviced */
  uvc_stop_streaming(devh);
  puts("Done streaming.");

  /* Release our handle on the device */
  uvc_close(devh);
  puts("Device closed");

  /* Release the device descriptor */
  uvc_unref_device(dev);

#if 0
      if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
      } else {
        /* Start the video stream. The library will call user function cb:
         *   cb(frame, (void *) 12345)
         */
        res = uvc_start_streaming(devh, &ctrl, cb, (void *) 12345, 0);

        if (res < 0) {
          uvc_perror(res, "start_streaming"); /* unable to start stream */
        } else {
          puts("Streaming...");

          /* enable auto exposure - see uvc_set_ae_mode documentation */
          puts("Enabling auto exposure ...");
          const uint8_t UVC_AUTO_EXPOSURE_MODE_AUTO = 2;
          res = uvc_set_ae_mode(devh, UVC_AUTO_EXPOSURE_MODE_AUTO);
          if (res == UVC_SUCCESS) {
            puts(" ... enabled auto exposure");
          } else if (res == UVC_ERROR_PIPE) {
            /* this error indicates that the camera does not support the full AE mode;
             * try again, using aperture priority mode (fixed aperture, variable exposure time) */
            puts(" ... full AE not supported, trying aperture priority mode");
            const uint8_t UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY = 8;
            res = uvc_set_ae_mode(devh, UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY);
            if (res < 0) {
              uvc_perror(res, " ... uvc_set_ae_mode failed to enable aperture priority mode");
            } else {
              puts(" ... enabled aperture priority auto exposure mode");
            }
          } else {
            uvc_perror(res, " ... uvc_set_ae_mode failed to enable auto exposure mode");
          }

          sleep(10); /* stream for 10 seconds */

          /* End the stream. Blocks until last callback is serviced */
          uvc_stop_streaming(devh);
          puts("Done streaming.");
        }
      }

      /* Release our handle on the device */
      uvc_close(devh);
      puts("Device closed");
    }

    /* Release the device descriptor */
    uvc_unref_device(dev);
   
  }
#endif

  /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
  uvc_exit(ctx);
  puts("UVC exited");

  return 0;
}
