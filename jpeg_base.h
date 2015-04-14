
#pragma once

#include <jpeglib.h>

struct jpeg_base
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr        jerr;
	struct jpeg_destination_mgr  dmgr;
};


void jpeg_base_init(struct jpeg_base* jpeg, int quality);
void jpeg_base_free(struct jpeg_base* jpeg);

/* Returns the size */
int  jpeg_base_compress(struct jpeg_base* jpeg, void* ibuf, void* obuf, int width, int height);