#pragma once
#include "frame.h"

/*UI element. Must be allocated in dynamic memory.*/
struct image_view
{
	frame fm;
	/*Can be nullptr.*/
	bitmap *image;
	/*Lower left point of image is (0,0) and upper right is (width, height).*/
	matrix<float32, 3, 3> transform;

	image_view();
};
