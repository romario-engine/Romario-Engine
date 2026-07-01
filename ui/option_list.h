#pragma once
#include "ui/button.h"
#include "ui/grid_layout.h"
#include "data/string.h"

enum struct option_list_open_anchor
{
	left,
	right,
	bottom,
	top
};

enum struct option_list_open_upon_action
{
	none,
	click,
	hover
};

/*UI element. Must be allocated in dynamic memory.*/
struct option_list
{
	button button;
	grid_layout list;
	/*Side at which list will be appered when opened at frame.*/
	option_list_open_anchor open_anchor;
	/*This value must not be changed manually.*/
	option_list_open_upon_action open_upon_action;
	function<void()> button_click_callback;
	function<void()> button_hover_callback;

	option_list();
	void set_opening(option_list_open_upon_action opening);
	/*<point> may be adjusted.
	Closes option list subtree starting from neighbor of option list button
	or closes entire tree if such neighbor does not exist.*/
	void open_at_point(vector<int32, 2> point);
	/*Open list at <fm> side stated by <open_anchor>.
	This point may be adjusted.
	Closes option list subtree starting from neighbor of option list button
	or closes entire tree if such neighbor does not exist.*/
	void open_at_frame(frame *fm);
	/*Closes option list subtree starting from this list.*/
	void close();
};
