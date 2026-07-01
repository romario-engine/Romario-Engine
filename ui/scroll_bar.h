#pragma once
#include "ui/frame.h"

enum struct scroll_bar_orientation
{
	vertical,
	horizontal
};

/*UI element. Must be allocated in dynamic memory.*/
struct scroll_bar
{
	frame fm;
	scroll_bar_orientation orientation;
	int32 min_slider_size;
	int32 roll_delta;
	uint32 content_size;
	uint32 viewport_size;
	uint32 viewport_offset;

	scroll_bar(scroll_bar_orientation type);
	void shift(int32 value);
	rectangle<int32> slider_rectangle(rectangle<int32> viewport);
};
