#pragma once

int demosaic_setup(struct cl_base* cl, cl_kernel kernel, size_t width, size_t height);
char* demosaic_run(struct cl_base* cl, cl_kernel kernel, const void* raw);
int demosaic_cleanup();
