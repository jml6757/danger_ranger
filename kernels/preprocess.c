#include <CL/cl.h>
#include <stdio.h>

#include "cl_base.h"

/* Buffers */
static cl_mem ibuf;                 /* Device input buffer */
static cl_mem obuf;                 /* Device output buffer */
static char* img;                   /* Host output buffer */
static unsigned int size;           /* Total image size */

int preprocess_init(struct cl_base* cl, struct cl_task* ts)
{
	int err;

	/* Build the kernel and set default parameters */
	cl_task_init(ts, cl, "kernels/preprocess.cl", "preprocess");

	/* Set local width and height variables */
	size = ts->g_size[0] * ts->g_size[1] * sizeof(short);

	/* Allocate space for an RGBA image */
	img = (char*) malloc(size);

	/* Create input buffer on GPU device */
	ibuf = clCreateBuffer(cl->context,                /* context               */
						  CL_MEM_READ_ONLY,           /* flags                 */
						  size,                       /* size                  */
						  NULL,                       /* host pointer          */
						  &err);                      /* error code (return)   */
	CL_CHECK(err, "Create Buffer (ibuf)");

	/* Create output buffer on GPU device */
	obuf = clCreateBuffer(cl->context,                /* context               */    
						  CL_MEM_READ_WRITE,          /* flags                 */    
						  size,                       /* size                  */    
						  NULL,                       /* host pointer         */    
						  &err);                      /* error code (return)   */  
	CL_CHECK(err, "Create Buffer (obuf)");

	/* Set kernel arguments */
	err = clSetKernelArg(ts->kernel, 0, sizeof(cl_mem), &ibuf);
	CL_CHECK(err, "Set Kernel Arg 0");
	err = clSetKernelArg(ts->kernel, 1, sizeof(cl_mem), &obuf);
	CL_CHECK(err, "Set Kernel Arg 1");

	return 0;
}

char* preprocess_run(struct cl_base* cl, struct cl_task* ts, const void* raw)
{
	int err;
	cl_event event;

	/* Copy image to GPU */
	err = clEnqueueWriteBuffer(cl->queue,     /* command_queue           */
							   ibuf,          /* buffer                  */
							   CL_TRUE,       /* blocking_read           */
							   0,             /* offset                  */
							   size,          /* cb                      */
							   raw,           /* ptr                     */
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
								 NULL);     /* event                   */
	CL_CHECK(err, "Enqueue Kernel");

	/* Copy image from GPU */
	err = clEnqueueReadBuffer(cl->queue,      /* command_queue           */
							  obuf,           /* buffer                  */
							  CL_TRUE,        /* blocking_read           */
							  0,              /* offset                  */
							  size,           /* cb                      */
							  img,            /* ptr                     */
							  0,              /* num_events_in_wait_list */
							  NULL,           /* event_wait_list         */
							  &event);        /* event                   */
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
	return img;
}

int preprocess_free()
{
	clReleaseMemObject(ibuf);
	clReleaseMemObject(obuf);
	return 0;
}
