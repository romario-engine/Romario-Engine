#pragma once
#include "ui/frame.h"
#include "ui/scroll_bar.h"

struct grid_layout_frame
{
	frame *fm;
	horizontal_align halign;
	vertical_align valign;

	grid_layout_frame() {}
	grid_layout_frame(
		frame *fm,
		horizontal_align halign = horizontal_align::center,
		vertical_align valign = vertical_align::center)
		: fm(fm), halign(halign), valign(valign) {}
};

/*UI element. Must be allocated in dynamic memory.*/
struct grid_layout
{
	frame fm;
	array<ui_size<float32>> rows_desc;
	array<ui_size<float32>> columns_desc;
	dynamic_matrix<grid_layout_frame> frames;
	uint64 growth_row;
	uint64 growth_column;
	scroll_bar xscroll;
	scroll_bar yscroll;

	grid_layout();
	void insert_row(uint32 insert_idx, ui_size<float32> size);
	void remove_row(uint32 remove_idx);
	void insert_column(uint32 insert_idx, ui_size<float32> size);
	void remove_column(uint32 remove_idx);
	void update_layout();
};
