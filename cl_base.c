#include <CL/cl.h>
#include "cl_base.h"

/* Copied from CLEW */
const char* cl_error(cl_int error)
{
    static const char* strings[] =
    {
        /* Error Codes*/
          "CL_SUCCESS"                                  /*  0  */
        , "CL_DEVICE_NOT_FOUND"                         /* -1  */
        , "CL_DEVICE_NOT_AVAILABLE"                     /* -2  */
        , "CL_COMPILER_NOT_AVAILABLE"                   /* -3  */
        , "CL_MEM_OBJECT_ALLOCATION_FAILURE"            /* -4  */
        , "CL_OUT_OF_RESOURCES"                         /* -5  */
        , "CL_OUT_OF_HOST_MEMORY"                       /* -6  */
        , "CL_PROFILING_INFO_NOT_AVAILABLE"             /* -7  */
        , "CL_MEM_COPY_OVERLAP"                         /* -8  */
        , "CL_IMAGE_FORMAT_MISMATCH"                    /* -9  */
        , "CL_IMAGE_FORMAT_NOT_SUPPORTED"               /* -10 */
        , "CL_BUILD_PROGRAM_FAILURE"                    /* -11 */
        , "CL_MAP_FAILURE"                              /* -12 */

        , "CL_UNKNOWN_ERROR_CODE"                       /* -13 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -14 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -15 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -16 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -17 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -18 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -19 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -20 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -21 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -22 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -23 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -24 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -25 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -26 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -27 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -28 */
        , "CL_UNKNOWN_ERROR_CODE"                       /* -29 */

        , "CL_INVALID_VALUE"                            /* -30 */
        , "CL_INVALID_DEVICE_TYPE"                      /* -31 */
        , "CL_INVALID_PLATFORM"                         /* -32 */
        , "CL_INVALID_DEVICE"                           /* -33 */
        , "CL_INVALID_CONTEXT"                          /* -34 */
        , "CL_INVALID_QUEUE_PROPERTIES"                 /* -35 */
        , "CL_INVALID_COMMAND_QUEUE"                    /* -36 */
        , "CL_INVALID_HOST_PTR"                         /* -37 */
        , "CL_INVALID_MEM_OBJECT"                       /* -38 */
        , "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"          /* -39 */
        , "CL_INVALID_IMAGE_SIZE"                       /* -40 */
        , "CL_INVALID_SAMPLER"                          /* -41 */
        , "CL_INVALID_BINARY"                           /* -42 */
        , "CL_INVALID_BUILD_OPTIONS"                    /* -43 */
        , "CL_INVALID_PROGRAM"                          /* -44 */
        , "CL_INVALID_PROGRAM_EXECUTABLE"               /* -45 */
        , "CL_INVALID_KERNEL_NAME"                      /* -46 */
        , "CL_INVALID_KERNEL_DEFINITION"                /* -47 */
        , "CL_INVALID_KERNEL"                           /* -48 */
        , "CL_INVALID_ARG_INDEX"                        /* -49 */
        , "CL_INVALID_ARG_VALUE"                        /* -50 */
        , "CL_INVALID_ARG_SIZE"                         /* -51 */
        , "CL_INVALID_KERNEL_ARGS"                      /* -52 */
        , "CL_INVALID_WORK_DIMENSION"                   /* -53 */
        , "CL_INVALID_WORK_GROUP_SIZE"                  /* -54 */
        , "CL_INVALID_WORK_ITEM_SIZE"                   /* -55 */
        , "CL_INVALID_GLOBAL_OFFSET"                    /* -56 */
        , "CL_INVALID_EVENT_WAIT_LIST"                  /* -57 */
        , "CL_INVALID_EVENT"                            /* -58 */
        , "CL_INVALID_OPERATION"                        /* -59 */
        , "CL_INVALID_GL_OBJECT"                        /* -60 */
        , "CL_INVALID_BUFFER_SIZE"                      /* -61 */
        , "CL_INVALID_MIP_LEVEL"                        /* -62 */
        , "CL_INVALID_GLOBAL_WORK_SIZE"                 /* -63 */
        , "CL_UNKNOWN_ERROR_CODE"
    };

    if (error >= -63 && error <= 0)
         return strings[-error];
    else
         return strings[64];
}

void cl_base_init(struct cl_base* cl)
{
	cl_int err = 0;
	cl_uint num_plats;

	/* Identify the platform */
	err = clGetPlatformIDs(1,                      /* num entries            */
						   &cl->platform,          /* platforms (return)     */
						   &num_plats);            /* num platforms (return) */
	CL_CHECK(err, "Get Platform ID");

	/* Access the GPU device */
	err = clGetDeviceIDs(cl->platform,             /* platform               */
						 CL_DEVICE_TYPE_GPU,       /* device type            */
						 1,                        /* num entries            */
						 &cl->device,              /* devices (return)       */
						 NULL);                    /* num devices (return)   */
	CL_CHECK(err, "Get Device ID");

	/* Create a context for the GPU */
	cl->context = clCreateContext(NULL,            /* context properties     */
								  1,               /* num devices            */
								  &cl->device,     /* devices                */
								  NULL,            /* callback funcion       */
								  NULL,            /* callback data          */
								  &err);           /* error code (return)    */
	CL_CHECK(err, "Create Context");

   /* Create a command queue for the GPU device */
	cl->queue = clCreateCommandQueue(cl->context,  /* context               */
									 cl->device,   /* device                */ 
									  CL_QUEUE_PROFILING_ENABLE ,            /* queue properties      */
									 &err);        /* error code (return)   */
	CL_CHECK(err, "Create Command Queue");
	cl->buffers = NULL;
	cl->buf_count = 0;
}

void cl_base_info(struct cl_base* cl)
{
	cl_int err;
	int i;

	cl_char   name[1024] = {0};
	cl_char vendor[1024] = {0};
	cl_device_type type;
	cl_uint comp_units;


	cl_uint max_work_itm_dims = 0;
	size_t  max_wrkgrp_size = 0;
	size_t *max_loc_size = 0;

	/* Pull all information */
	err = clGetDeviceInfo(cl->device, CL_DEVICE_NAME, sizeof(name), &name, NULL);
	CL_CHECK(err, "Device Info (Name)");

	err = clGetDeviceInfo(cl->device, CL_DEVICE_VENDOR, sizeof(name), &name, NULL);
	CL_CHECK(err, "Device Info (Vendor)");

	err = clGetDeviceInfo(cl->device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
	CL_CHECK(err, "Device Info (Type)");

	err = clGetDeviceInfo(cl->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &comp_units, NULL);
	CL_CHECK(err, "Device Info (Compute Units)");

	err = clGetDeviceInfo( cl->device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &max_work_itm_dims, NULL);
	CL_CHECK(err, "Device Info (Max Work Item Dimensions)");

	max_loc_size = malloc(sizeof(size_t) * max_work_itm_dims);
	
	err = clGetDeviceInfo( cl->device, CL_DEVICE_MAX_WORK_ITEM_SIZES, max_work_itm_dims * sizeof(size_t), max_loc_size, NULL);
	CL_CHECK(err, "Device Info (Max Work Item Sizes)");

	err = clGetDeviceInfo( cl->device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_wrkgrp_size, NULL);
	CL_CHECK(err, "Device Info (Max Work Group Sizes)");

	printf("------------------------- CL Device Information --------------------------\n");
	/* Display all information */
	printf("Name: %s\n", name);
	printf("Vendor: %s\n", vendor);

	switch(type)
	{
		case CL_DEVICE_TYPE_GPU:
			printf("Type: GPU\n");
			break;
		case CL_DEVICE_TYPE_CPU:
			printf("Type: CPU\n");
			break;
		default:
			printf("Type: Unknown\n");
			break;
	}

	printf("Compute Units: %d\n",comp_units);

	printf("Maximum Local Dimensions: ( ");
	for(i = 0; i < max_work_itm_dims; ++i)
	{
		printf("%d ",(int) max_loc_size[i]);
	}
	printf(")\n");

	printf("Maximum Work Group Size: %d ",(int) max_wrkgrp_size);
	for(i = 0; (1 << i) < (max_wrkgrp_size >> i); i++);
	printf("( %d %d )\n", (1 << i), (int) (max_wrkgrp_size >> i));
	printf("--------------------------------------------------------------------------\n");
}

void cl_base_free(struct cl_base* cl)
{
	int i;
	int err;

	clReleaseContext(cl->context);
	clReleaseCommandQueue(cl->queue);
	if(cl->buffers != NULL)
	{
		for(i = 0; i < cl->buf_count; ++i)
		{
			err = clReleaseMemObject(cl->buffers[i]);
			CL_CHECK(err, "Release Memory Object");
		}

		free(cl->buffers);
	}
}

void* cl_base_mem_alloc(struct cl_base* cl, int size, int flags)
{
	cl_mem mem;
	void* ptr;
	int err;
	
	mem = clCreateBuffer(cl->context,                    /* context      */
						 flags | CL_MEM_ALLOC_HOST_PTR,  /* flags        */
						 size,                           /* size         */
						 NULL,                           /* user pointer */
						 &err);                          /* error        */
	CL_CHECK(err, "Memory Allocation");

 	ptr = clEnqueueMapBuffer(cl->queue,     /* queue                   */
 							 mem,           /* memory buffer           */
 							 CL_TRUE,       /* blocking call           */
 							 CL_MAP_WRITE,  /* flags                   */
 							 0,             /* offset                  */
 							 size,          /* cb                      */
 							 0,             /* num_events_in_wait_list */
 							 NULL,          /* event_wait_list         */
 							 NULL,          /* event                   */
 							 &err);         /* error                   */
 	CL_CHECK(err, "Memory Map");


	return ptr;
}

void cl_base_mem_free(struct cl_base* cl, cl_mem mem, void* ptr)
{
	//TODO
}



void cl_task_init(struct cl_task* ts, struct cl_base* cl, const char* filename, const char* function)
{
	cl_int err;
	FILE *file;
	char *buf;
	char *log;
	size_t size;

	/* Set global and local sizes (hardcoded for this application)*/
	ts->g_size[0] = 640;
	ts->g_size[1] = 480;
	ts->l_size[0] = 16;
	ts->l_size[1] = 16; 

	/* Open program file */
	file = fopen(filename, "r");
	if(file == NULL) {
		perror("File Open");
		exit(1);
	}

	/* Determine file size */
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	/* Read file into buffer */
	buf = (char*) calloc(size + 1, sizeof(char));
	fread(buf, sizeof(char), size, file);

	/* Create program from the file buffer */
	ts->program = clCreateProgramWithSource(cl->context,         /* context               */
											1,                   /* number of buffers     */
											(const char**) &buf, /* code buffers          */
											&size,               /* code buffer sizes     */
											&err);               /* error code (return)   */
	CL_CHECK(err, "Create Program");

	/* Build the program */
	err = clBuildProgram(ts->program,  /* program           */
						 0,            /* num devices       */
						 NULL,         /* device list       */
						 NULL,         /* options           */
						 NULL,         /* callback function */
						 NULL);        /* callback data     */
	if(err < 0)
	{
		/* Get build information*/
		err = clGetProgramBuildInfo(ts->program, cl->device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &size);
		CL_CHECK(err, "Build Info Size");
		log = malloc(size + 1);

		err = clGetProgramBuildInfo(ts->program, cl->device, CL_PROGRAM_BUILD_LOG, size, log, NULL);
		CL_CHECK(err, "Build Info Log");
		
		/* Display information*/
		printf("%s", log);
		
		/* Cleanup */
		free(log);
		exit(1);
	}

	/* Create the kernel */
	ts->kernel = clCreateKernel(ts->program,      /* program              */
								function,         /* kernel function name */
								&err);            /* error code (return)  */
	CL_CHECK(err, "Create Kernel");

	/* Clean up */
	fclose(file);
	free(buf);
}

void cl_task_free(struct cl_task* ts)
{
	clReleaseKernel(ts->kernel);	
	clReleaseProgram(ts->program);
}

#if 0
int main() 
{
	struct cl_base cl;
	struct cl_task ts;

	/* Initialize GPU */
	cl_base_init(&cl);

	/* Display GPU information */
	cl_base_info(&cl);

	/* Build/Run kernel here*/
	cl_task_init(&ts, &cl, "preprocess.cl", "preprocess");

	/* Cleanup */
	cl_task_free(&ts);
	cl_base_free(&cl);

	return 0;
}
#endif