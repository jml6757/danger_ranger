#pragma once

struct short2
{
	short x;
	short y;
};

struct points
{
	int count;
	struct short2 *coords;
};

void fast_init(struct cl_base* cl, struct cl_task* ts, cl_mem image, cl_mem table, cl_mem point);
struct points fast_run(struct cl_base* cl, struct cl_task* ts, char* l_image);
void fast_free();
