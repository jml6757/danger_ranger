
#define MULTIPLIER 15

/******************************************************************************
 * Description: Takes in a 16 bit grayscale image at the source input and 
 * outputs a 8 bit grayscale image that is stretched by the multiplier
 * constant
 *
 * @param   src   16-bit grayscale input image
 * @param   img   8-bit grayscale adjusted output image
 *****************************************************************************/
__kernel void preprocess(__global read_only  uchar*  src,
						 __global write_only uchar*  img)
{
	/* Get pixel index */
	int idx = get_global_id(1)*get_global_size(0) + get_global_id(0);
	
	/* Cast to 16-bit buffer */
	ushort* pixel = src;

	/* Scale pixel intensity and write to output buffer*/
	img[idx] = ((float) pixel[idx] * MULTIPLIER / (float) 0xFFFF) * (float) 255;
}
