/**
 * @brief    V4L Wrapper Class
 * @details  Provides structures and wrapper functions to perform all functions
 *           necessary for camera capture with the use of user allocated
 *           pointers.
 */

#pragma once

#include <stdio.h>             /* perror()            */
#include <stdlib.h>            /* exit()              */
#include <linux/videodev2.h>   /* sturct v4l2_buffer  */

#define V4L_MAX_IMGS 5
#define V4L_CHECK(err, msg) if(err < 0) {perror(msg); exit(1);}

/**
 * @brief    V4L Device Information
 * @details  Contains all information required for V4L operations for a
 *           specified device.
 */
struct v4l_base 
{
	int fd;                                /**< Descriptor for V4L device    */
	int count;                             /**< Number of image buffers      */
	struct v4l2_buffer bufs[V4L_MAX_IMGS]; /**< List of image buffers        */
};

/*---------------------------- General Functions ----------------------------*/

int v4l_base_init(struct v4l_base* v4l, const char* device, const int width, const int height);

int v4l_base_free(struct v4l_base* v4l);

int v4l_base_info(struct v4l_base* v4l);

/*---------------------------- Memory Functions -----------------------------*/

int v4l_base_mem_add(struct v4l_base* v4l, void* ptr, int length);

/*---------------------------- Capture Functions ----------------------------*/

int v4l_base_capture_start(struct v4l_base* v4l);

int v4l_base_capture_stop(struct v4l_base* v4l);

void v4l_base_enqueue(struct v4l_base* v4l, struct v4l2_buffer* buf);

struct v4l2_buffer* v4l_base_dequeue(struct v4l_base* v4l);

