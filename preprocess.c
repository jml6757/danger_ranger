#include <CL/cl.h>
#include <stdio.h>

#include "cl_base.h"

/* Buffers */
static cl_mem ibuf;                 /* Device input buffer */
static cl_mem obuf;                 /* Device output buffer */
static char* img;                   /* Host output buffer */

/* Sizes */
static size_t g_size[2];            /* Global image size */
static size_t l_size[2] = {16, 16}; /* Workgroup size (platform dependent) */
static unsigned int size;           /* Total image size */

int preprocess_init(struct cl_base* cl, cl_kernel kernel, size_t width, size_t height)
{
	int err;

	/* Set local width and height variables */
	g_size[0] = width;
	g_size[1] = height;
	size = width * height * sizeof(short);

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
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &ibuf);
	CL_CHECK(err, "Set Kernel Arg 0");
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &obuf);
	CL_CHECK(err, "Set Kernel Arg 1");

	return 0;
}

char* preprocess_run(struct cl_base* cl, cl_kernel kernel, const void* raw)
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
								 kernel,      /* kernel                  */
								 2,           /* work_dim                */
								 NULL,        /* global_work_offset      */
								 (const size_t *)&g_size,     /* global_work_size        */
								 (const size_t *)&l_size,     /* local_work_size         */
								 0,           /* num_events_in_wait_list */
								 NULL,        /* event_wait_list         */
								 &event);     /* event                   */
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
							  NULL);          /* event                   */
	CL_CHECK(err, "Read Buffer");

#if 0
	/* Wait for computation kernel to finish*/
	clWaitForEvents(1, &event);

	/* Obtain the start- and end time for the event */
	unsigned long start = 0;
	unsigned long end = 0;
	err = clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_START, sizeof(cl_ulong),&start,NULL);       
	err = clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_END, sizeof(cl_ulong),&end,NULL);

	/* Compute the duration in nanoseconds */
	unsigned long duration = end - start;

	printf("Computation Time: %luns\n", duration);
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
