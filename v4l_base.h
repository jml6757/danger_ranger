#pragma once

#define V4L_MAX_IMGS 4
#define V4L_CHECK(err, msg) if(err < 0) {perror(msg); exit(1);}

struct v4l_img 
{
	void *start;
	size_t length;
};

struct v4l_base 
{
	int fd;
	struct v4l_img img[V4L_MAX_IMGS];
};

int v4l_base_init(struct v4l_base* v4l, const char* device, const int width, const int height);
int v4l_base_info(struct v4l_base* v4l);

int v4l_base_capture_start(struct v4l_base* v4l);
int v4l_base_capture_stop(struct v4l_base* v4l);

struct v4l_img* v4l_base_read(struct v4l_base* v4l);
int v4l_base_free(struct v4l_base* v4l);