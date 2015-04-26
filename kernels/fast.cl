#define MAX_PTS 1024
#define fetch(x,y) image[(y)*width + x]

__kernel void fast(__global read_only  uchar*    image,
				   __constant          uchar*    table,
				   __global            ushort2*  point,
				   __global            uint*     count)
{	
	/* Coordinate data*/
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint width = get_global_size(0);             
	uint height = get_global_size(1);

	/* Constant data */
	const uchar16 threshold = 20;
	const char16 one = 0x01;

	/* Center pixel */
	uchar16 center = fetch(x, y);

	/* 3x3 ring surrounding the center pixel */
	uchar16 ring = 
	{
		fetch(x - 1, y - 3), // 0
		fetch(x + 1, y + 3), // 8
		fetch(x - 3, y + 1), // 4
		fetch(x + 3, y - 1), // 12
		fetch(x - 2, y - 2), // 1
		fetch(x + 2, y + 2), // 9
		fetch(x - 2, y + 2), // 5		
		fetch(x + 2, y - 2), // 13
		fetch(x - 3, y - 1), // 2
		fetch(x + 3, y + 1), // 10
		fetch(x - 1, y + 3), // 6
		fetch(x + 1, y - 3), // 14
		fetch(x - 3, y),     // 3
		fetch(x + 3, y),     // 11
		fetch(x, y + 3),     // 7
		fetch(x, y - 3)      // 15
	};

	/* Calculate which pixels are above or below the threshold */
	char16 gt_thresh = abs_diff(ring, center) >= threshold;

	/* Calculate which pixels are greater than the center */
	char16 gt_center = ring > center;

	/* Pixels which are greater than the threshold and are greater than the center are whites */
	char16 whites = gt_thresh & gt_center & one;

	/* Pixels which are greater than the threshold and are less than the center are blacks */
	char16 blacks = gt_thresh & ~gt_center & one;

	/* Compress bright/dark flags into a short */
	uint4 tmp;
	uint w_flags;
	uint b_flags;

	/* Whites */
	tmp = as_uint4(whites);
	w_flags = tmp.x | (tmp.y << 1) | (tmp.z << 2) | (tmp.w << 3);
	w_flags = (w_flags & 0x0000FFFF) | ((w_flags >> 16) << 4);

	/* Blacks */
	tmp = as_uint4(blacks);
	b_flags = tmp.x | (tmp.y << 1) | (tmp.z << 2) | (tmp.w << 3);
	b_flags = (w_flags & 0x0000FFFF) | ((w_flags >> 16) << 4);

	/* Return the keypoint */
	if((table[(w_flags >> 3) - 63] & (1 << (w_flags & 7))) || 
	   (table[(b_flags >> 3) - 63] & (1 << (b_flags & 7))))
	{
		if(*count < MAX_PTS)
		{
			 uint index = atomic_inc(count);
			 point[index].x = x;
			 point[index].y = y;
		}
	}
}