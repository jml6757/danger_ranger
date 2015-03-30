#pragma once

int preprocess_init(struct cl_base* cl, cl_kernel kernel, size_t width, size_t height);
char* preprocess_run(struct cl_base* cl, cl_kernel kernel, const void* raw);
int preprocess_free();
