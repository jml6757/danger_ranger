
#include <stdio.h>
#include "jpeg_base.h"

/* setup the buffer but we did that in the main function */
static void init_buffer(struct jpeg_compress_struct* cinfo) {}
 
/* What to do when the buffer is full; this should almost never
 * happen since we allocated our buffer to be big to start with
 */
static boolean empty_buffer(struct jpeg_compress_struct* cinfo) 
{
	return TRUE;
}
 
/* Finalize the buffer and do any cleanup stuff */
static void term_buffer(struct jpeg_compress_struct* cinfo) {}

void jpeg_base_init(struct jpeg_base* jpeg, int quality)
{
	/* Initialize structures */
	jpeg->cinfo.err = jpeg_std_error(&jpeg->jerr);
	jpeg_create_compress(&jpeg->cinfo);

	/* Set destination parameters */
	jpeg->dmgr.init_destination    = init_buffer;
	jpeg->dmgr.empty_output_buffer = empty_buffer;
	jpeg->dmgr.term_destination    = term_buffer;
	//jpeg->dmgr.next_output_byte    = jpeg->output;
	//jpeg->dmgr.free_in_buffer      = width * height;

	/* Set compression parameters */
	jpeg->cinfo.dest             = &jpeg->dmgr;
	jpeg->cinfo.input_components = 1;
	jpeg->cinfo.in_color_space   = JCS_GRAYSCALE;
	jpeg_set_defaults(&jpeg->cinfo);
	jpeg->cinfo.dct_method       = JDCT_IFAST;

	/* Set quality */
	jpeg_set_quality(&jpeg->cinfo, quality, TRUE);
}

void jpeg_base_free(struct jpeg_base* jpeg)
{
	jpeg_destroy_compress(&(jpeg->cinfo));
}

int jpeg_base_compress(struct jpeg_base* jpeg, void* ibuf, void* obuf, int width, int height)
{
	JSAMPROW row_pointer;

	/* Set dimensions */
	jpeg->cinfo.image_width                = width;
	jpeg->cinfo.image_height               = height;

	/* Reset buffer position */
	jpeg->cinfo.dest->next_output_byte    = obuf;
	jpeg->cinfo.dest->free_in_buffer      = width * height;

	/* Perform compression */
	jpeg_start_compress(&jpeg->cinfo, TRUE);
    while(jpeg->cinfo.next_scanline < jpeg->cinfo.image_height )
    {
        row_pointer = (JSAMPROW) &ibuf[jpeg->cinfo.next_scanline * width];
        jpeg_write_scanlines(&jpeg->cinfo, &row_pointer, 1);
    }
    
    /* Cleanup */
    jpeg_finish_compress(&jpeg->cinfo);

    return ((void*) jpeg->cinfo.dest->next_output_byte) - obuf;
}