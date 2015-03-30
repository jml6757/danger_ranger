#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

struct cl_base
{
	cl_mem* buffers;
	int buf_count;
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
};

#define CL_CHECK(err, msg) if(err < 0) {fprintf(stderr, "%s: %s(%d)\n", msg, cl_error(err), err); exit(1);}

const char* cl_error(cl_int error);

void cl_base_init(struct cl_base* cl);
void cl_base_info(struct cl_base* cl);
void cl_base_free(struct cl_base* cl);

cl_kernel cl_kernel_init(struct cl_base* cl, const char* filename, const char* entry);
void cl_kernel_free(cl_kernel kernel);