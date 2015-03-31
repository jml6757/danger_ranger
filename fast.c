#include <CL/cl.h>
#include <stdio.h>

#include "cl_base.h"

/* Buffers */
static cl_mem _image;
static cl_mem _table;
static cl_mem _point;
static short l_points[256];
static char  l_table[8196];

void fast_init(struct cl_base* cl, struct cl_task* ts, cl_mem image, cl_mem table, cl_mem point)
{
	int err;

	/* Create local copies*/
	_image = image;
	_table = table;
	_point = point;

	/* Build the kernel and set default parameters */
	cl_task_init(ts, cl, "fast.cl", "fast");

	/* Set kernel arguments */
	err = clSetKernelArg(ts->kernel, 0, sizeof(cl_mem), &image);
	CL_CHECK(err, "Set Kernel Arg 0 (Image)");
	err = clSetKernelArg(ts->kernel, 1, sizeof(cl_mem), &table);
	CL_CHECK(err, "Set Kernel Arg 1 (Table)");
	err = clSetKernelArg(ts->kernel, 2, sizeof(cl_mem), &point);
	CL_CHECK(err, "Set Kernel Arg 2 (Point)");
	err = clSetKernelArg(ts->kernel, 3, sizeof(int), 0);
	CL_CHECK(err, "Set Kernel Arg 3 (Count)");
}

short* fast_run(struct cl_base* cl, struct cl_task* ts)
{
	int err;
	cl_event event;

	/* Copy table to GPU (Size is hardcoded for testing purposes, mali uses shared memory) */
	err = clEnqueueWriteBuffer(cl->queue,     /* command_queue           */
							   _table,        /* buffer                  */
							   CL_TRUE,       /* blocking_read           */
							   0,             /* offset                  */
							   8196,          /* cb                      */
							   l_table,       /* ptr                     */
							   0,             /* num_events_in_wait_list */
							   NULL,          /* event_wait_list         */
							   NULL);         /* event                   */
	CL_CHECK(err, "Write Buffer");

	/* Enqueue kernel */
	err = clEnqueueNDRangeKernel(cl->queue,   /* command_queue           */
								 ts->kernel,  /* kernel                  */
								 2,           /* work_dim                */
								 NULL,        /* global_work_offset      */
								 &ts->g_size, /* global_work_size        */
								 &ts->l_size, /* local_work_size         */
								 0,           /* num_events_in_wait_list */
								 NULL,        /* event_wait_list         */
								 &event);     /* event                   */
	CL_CHECK(err, "Enqueue Kernel");

	/* Copy image from GPU (Size is hardcoded for testing purposes, mali uses shared memory) */
	err = clEnqueueReadBuffer(cl->queue,      /* command_queue           */
							  _point,         /* buffer                  */
							  CL_TRUE,        /* blocking_read           */
							  0,              /* offset                  */
							  256 * 2,        /* cb                      */
							  l_points,       /* ptr                     */
							  0,              /* num_events_in_wait_list */
							  NULL,           /* event_wait_list         */
							  NULL);          /* event                   */
	CL_CHECK(err, "Read Buffer");

	/* Wait for computation kernel to finish*/
	clWaitForEvents(1, &event);

#if 0
	/* Obtain the start- and end time for the event */
	unsigned long start = 0;
	unsigned long end = 0;
	err = clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_START, sizeof(cl_ulong),&start,NULL);       
	err = clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_END, sizeof(cl_ulong),&end,NULL);

	/* Compute the duration in microseconds */
	double duration = (double) (end - start) / 1000.0f;

	printf("Computation Time: %0.1lfus\n", duration);
#endif

	clReleaseEvent(event);
	return l_points;
}

void fast_free()
{
	return;
}
