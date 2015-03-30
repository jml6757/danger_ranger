/*

STUB IMPLEMENTATION - UNTESTED

*/

union converter
{
	char16* c;
	short8* s;
	int4*   i;
	long2*  l;
	uchar16* uc;
	ushort8* us;
	uint4*   ui;
	ulong2*  ul;
};

#define MAX_PTS 256
#define fetch(x,y) image[y*width + x]

__kernel void preprocess(__read_only   uchar*    image,
						 __constant    uchar*    table,
						 __global      ushort2*  point,
						 __global      uint      count)
{        
	union converter c;
	
	/* Coordinate data*/
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint width = get_global_size(0);             
	uint height = get_global_size(1);

	/* Constant data */
	uchar16 threshold = 40;

	/* Center pixel */
	uchar16 center = fetch(x, y);

	/* 3x3 ring surrounding the center pixel */
	uchar16 ring = 
	(
		fetch(x,y),  // 1 
		fetch(x,y),  // 2 
		fetch(x,y),  // 3 
		fetch(x,y),  // 4 
		fetch(x,y),  // 5 
		fetch(x,y),  // 6 
		fetch(x,y),  // 7 
		fetch(x,y),  // 8 
		fetch(x,y),  // 9 
		fetch(x,y),  // 10 
		fetch(x,y),  // 11 
		fetch(x,y),  // 12 
		fetch(x,y),  // 13 
		fetch(x,y),  // 14
		fetch(x,y),  // 15
		fetch(x,y)   // 16
	);

	/* Calculate which pixels are above or below the threshold */
	uchar16 gt_thresh = abs_diff(ring, center) > threshold;

	/* Calculate which pixels are greater than the center */
	uchar16 gt_center = ring > center;

	/* Pixels which are greater than the threshold and are greater than the center are whites */
	uchar16 whites = gt_thresh & gt_center;

	/* Pixels which are greater than the threshold and are less than the center are blacks */
	uchar16 blacks = gt_thresh & ~gt_center;
	
	uint num_whites = popcount(whites) >> 3;
	uint num_blacks = popcount(blacks) >> 3;

	/* Compress bright/dark flags into a short */
	c.c = &whites;
	c.l->x |= c.l->y << 1;
	c.i->x |= c.i->y << 1;
	c.s->x |= c.s->y << 1;
	ushort w_flags = c.s->x;

	c.c = &blacks;
	c.l->x |= c.l->y << 1;
	c.i->x |= c.i->y << 1;
	c.s->x |= c.s->y << 1;
	ushort b_flags = c.s->x;    
	
	/* Return the keypoint */
	if(table[w_flags >> 3] || table[b_flags >> 3])
	{
		if(count < MAX_PTS)
		{
			 uint index = atomic_inc(&count);
			 point[index] =  (short2)(x,y);
		}
	}
}