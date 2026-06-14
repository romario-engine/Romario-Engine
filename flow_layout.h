#pragma once
#include "frame.h"
#include "scroll_bar.h"

enum struct flow_axis
{
	x,
	y
};

enum struct flow_line_offset
{
	left,
	right
};

struct flow_layout_frame
{
	frame *fm;
	horizontal_align halign;
	vertical_align valign;
	bool line_break;

	flow_layout_frame() {}

	flow_layout_frame(
		frame *fm,
		horizontal_align halign = horizontal_align::left,
		vertical_align valign = vertical_align::top,
		bool line_break = false)
		: fm(fm),
		halign(halign),
		valign(valign),
		line_break(line_break) {}
};

/*UI element. Must be allocated in dynamic memory.*/
struct flow_layout
{
	frame fm;
	array<flow_layout_frame> frames;
	flow_axis direction;
	flow_line_offset offset;
	bool multiline;
	scroll_bar xscroll;
	scroll_bar yscroll;

	flow_layout();
	void update_layout();
};
