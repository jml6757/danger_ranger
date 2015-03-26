
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "v4l_base.h"

static int v4l_set_format(struct v4l_base* v4l, int width, int height)
{
	int err;

	/* Set video format parameters */
	struct v4l2_format fmt = {0};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	/* Set format on device */
	err = ioctl(v4l->fd, VIDIOC_S_FMT, &fmt);
	V4L_CHECK(err, "Set Format Error");

	return 0;
}

static int v4l_req_buffer(struct v4l_base* v4l, int count)
{
	int err, i;
	struct v4l2_buffer buffer;
	struct v4l_img* img;


	/* Create buffer request */
	struct v4l2_requestbuffers req = {0};
	req.count = count;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	
	/* Request buffer allocation */
	err = ioctl(v4l->fd, VIDIOC_REQBUFS, &req);
	V4L_CHECK(err, "Request Buffer Error");

	/* Memory map the requested buffers */
	for (i = 0; i < req.count; i++)
	{
		/* Set buffer parameters */ 
		buffer.type = req.type;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;

		/* Query driver buffer data */
		err = ioctl(v4l->fd, VIDIOC_QUERYBUF, &buffer);
		V4L_CHECK(err, "Query Buffer Error");

		/* Set userspace buffer data */
		img = &v4l->img[i];
		img->length = buffer.length;
		img->start  = mmap(NULL, buffer.length,
							  PROT_READ | PROT_WRITE,
							  MAP_SHARED,
							  v4l->fd, buffer.m.offset);

		V4L_CHECK(img->start, "Mmap Error");

		/* Put all buffers in the video queue */
		err = ioctl(v4l->fd, VIDIOC_QBUF, &buffer);
		V4L_CHECK(err, "Initial Buffer Queue Error");

	}
	return 0;
}

int v4l_base_init(struct v4l_base* v4l, const char* device, const int width, const int height)
{
	/* Open camera device */
	v4l->fd = open(device, O_RDWR);
	if(v4l->fd == 0)
	{
		perror("Device Open Error");
		return -1;
	}
	
	/* Initialize formatting and buffers */
	v4l_set_format(v4l, width, height);
	v4l_req_buffer(v4l, V4L_MAX_IMGS);
	return 0;
}

int v4l_base_info(struct v4l_base* v4l)
{
	int err;
	struct v4l2_capability cap;

	/* Query Capabilities */
	err = ioctl(v4l->fd, VIDIOC_QUERYCAP, &cap);
	V4L_CHECK(err, "Capabilities Query Error");

	/* Print Capabilities */

	printf("------------------------- V4L Device Information -------------------------\n");
	printf("Driver:    %s\n", cap.driver);
	printf("Device:    %s\n", cap.card);
	printf("Bus Info:  %s\n", cap.bus_info);
	printf("Version:   %u.%u.%u\n", 
		   (cap.version >> 16) & 0xFF, 
		   (cap.version >> 8)  & 0xFF, 
		   (cap.version)       & 0xFF);
	printf("Capabilities:\n");

	if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE        )
		printf("\tV4L2_CAP_VIDEO_CAPTURE       : Is a video capture device\n");
	if(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT         )
		printf("\tV4L2_CAP_VIDEO_OUTPUT        : Is a video output device\n");
	if(cap.capabilities & V4L2_CAP_VIDEO_OVERLAY        )
		printf("\tV4L2_CAP_VIDEO_OVERLAY       : Can do video overlay\n");
	if(cap.capabilities & V4L2_CAP_VBI_CAPTURE          )
		printf("\tV4L2_CAP_VBI_CAPTURE         : Is a raw VBI capture device\n");
	if(cap.capabilities & V4L2_CAP_VBI_OUTPUT           )
		printf("\tV4L2_CAP_VBI_OUTPUT          : Is a raw VBI output device\n");
	if(cap.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE   )
		printf("\tV4L2_CAP_SLICED_VBI_CAPTURE  : Is a sliced VBI capture device\n");
	if(cap.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT    )
		printf("\tV4L2_CAP_SLICED_VBI_OUTPUT   : Is a sliced VBI output device\n");
	if(cap.capabilities & V4L2_CAP_RDS_CAPTURE          )
		printf("\tV4L2_CAP_RDS_CAPTURE         : RDS data capture\n");
	if(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT_OVERLAY )
		printf("\tV4L2_CAP_VIDEO_OUTPUT_OVERLAY: Can do video output overlay\n");
	if(cap.capabilities & V4L2_CAP_HW_FREQ_SEEK         )
		printf("\tV4L2_CAP_HW_FREQ_SEEK        : Can do hardware frequency seek \n");
	if(cap.capabilities & V4L2_CAP_TUNER                )
		printf("\tV4L2_CAP_TUNER               : Has a tuner\n");
	if(cap.capabilities & V4L2_CAP_AUDIO                )
		printf("\tV4L2_CAP_AUDIO               : Has audio support\n");
	if(cap.capabilities & V4L2_CAP_RADIO                )
		printf("\tV4L2_CAP_RADIO               : Is a radio device\n");
	if(cap.capabilities & V4L2_CAP_READWRITE            )
		printf("\tV4L2_CAP_READWRITE           : Read/write systemcalls\n");
	if(cap.capabilities & V4L2_CAP_ASYNCIO              )
		printf("\tV4L2_CAP_ASYNCIO             : Async I/O\n");
	if(cap.capabilities & V4L2_CAP_STREAMING            )
		printf("\tV4L2_CAP_STREAMING           : Streaming I/O ioctls\n");
	printf("--------------------------------------------------------------------------\n");

	return 0;
}

int v4l_base_capture_start(struct v4l_base* v4l)
{
	int err;
	int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* Start streaming */
	err = ioctl(v4l->fd, VIDIOC_STREAMON, &stream_type);
	V4L_CHECK(err, "Stream On Error");
	return 0;
}

int v4l_base_capture_stop(struct v4l_base* v4l)
{
	int err;
	int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* Stop streaming */
	err = ioctl(v4l->fd, VIDIOC_STREAMOFF, &stream_type);
	V4L_CHECK(err, "Stream On Error");
	return 0;
}

struct v4l_img* v4l_base_read(struct v4l_base* v4l)
{
	int err;
	struct v4l2_buffer buffer = {0};
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* Remove buffer from front of queue (BLOCKING) */
	err = ioctl(v4l->fd, VIDIOC_DQBUF, &buffer);
	V4L_CHECK(err, "Buffer Dequeue Error");

	/* Add buffer back to the end of queue*/
	err = ioctl(v4l->fd, VIDIOC_QBUF, &buffer);
	V4L_CHECK(err, "Buffer Queue Error");

	return &(v4l->img[buffer.index]);
}

int v4l_base_free(struct v4l_base* v4l)
{
	/* TODO: Implement proper cleanup code */
	close(v4l->fd);
	return 0;
}

#if 0
int main()
{
	struct v4l_base v4l;

	/* Initialize video device */
	v4l_base_init(&v4l, "/dev/video0", 640, 480);

	/* Display device information*/
	v4l_base_info(&v4l);

	/* Start image capture */
	v4l_base_capture_start(&v4l);

	/* Stop image capture */
	v4l_base_capture_stop(&v4l);

	/* Cleanup */
	v4l_base_free(&v4l);

	return 0;
}
#endif