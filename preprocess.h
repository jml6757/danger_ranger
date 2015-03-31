#pragma once

int preprocess_init(struct cl_base* cl, struct cl_task* ts);
char* preprocess_run(struct cl_base* cl, struct cl_task* ts, const void* raw);
int preprocess_free();
