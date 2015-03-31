#pragma once

void fast_init(struct cl_base* cl, struct cl_task* ts, cl_mem image, cl_mem table, cl_mem point);
short* fast_run(struct cl_base* cl, struct cl_task* ts);
void fast_free();
