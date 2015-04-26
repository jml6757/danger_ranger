
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

static int v4l_base_set_format(struct v4l_base* v4l, int width, int height)
{
	int err;
	struct v4l2_format fmt = {0};

	/* Set video format parameters */
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

int v4l_base_mem_init(struct v4l_base* v4l, int count)
{
	int err;
	struct v4l2_requestbuffers req = {0};

	/* Create buffer request */
	req.count = count;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;
	
	/* Request buffer allocation */
	err = ioctl(v4l->fd, VIDIOC_REQBUFS, &req);
	V4L_CHECK(err, "Request Buffer Error");

	return 0;
}


/*---------------------------- General Functions ----------------------------*/

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
	v4l_base_set_format(v4l, width, height);
	v4l_base_mem_init(v4l, V4L_MAX_IMGS);
	return 0;
}

int v4l_base_free(struct v4l_base* v4l)
{
	/* TODO: Implement proper cleanup code */
	close(v4l->fd);
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


/*---------------------------- Memory Functions -----------------------------*/

int v4l_base_mem_add(struct v4l_base* v4l, void* ptr, int length)
{
	struct v4l2_buffer buffer;

	/* Set buffer parameters */ 
	buffer.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory    = V4L2_MEMORY_USERPTR;
	buffer.index     = v4l->count;
	buffer.m.userptr = (unsigned long) ptr;
	buffer.length    = length;

	/* Add buffer to local V4L data */
	v4l->bufs[v4l->count] = buffer;

	/* Increment total buffer count */
	v4l->count++;

	return 0;
}


/*---------------------------- Capture Functions ----------------------------*/

int v4l_base_capture_start(struct v4l_base* v4l)
{
	int err, i;
	int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* Add buffers to the capture queue */
	for (i = 0; i < v4l->count; ++i) 
	{
		err = ioctl(v4l->fd, VIDIOC_QBUF, &(v4l->bufs[i]));
		V4L_CHECK(err, "Queue Buffer Error");
	}

	/* Start streaming to buffers */
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

void v4l_base_enqueue(struct v4l_base* v4l, struct v4l2_buffer* buf)
{
	/* Add buffer back to the end of queue*/
	int err = ioctl(v4l->fd, VIDIOC_QBUF, buf);
	V4L_CHECK(err, "Buffer Queue Error");
}

struct v4l2_buffer* v4l_base_dequeue(struct v4l_base* v4l)
{
	int err;
	struct v4l2_buffer buffer = {0};
	
	/* Set buffer parameters */
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_USERPTR;

	/* Remove buffer from front of queue (BLOCKING) */
	err = ioctl(v4l->fd, VIDIOC_DQBUF, &buffer);
	V4L_CHECK(err, "Buffer Dequeue Error");

	/* Return local buffer pointer */
	return &(v4l->bufs[buffer.index]);
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