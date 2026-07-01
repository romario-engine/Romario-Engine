#pragma once
#include "global/global_types.h"
#include "data/array.h"
#include "ui/ui_types.h"
#include "graphics/graphics.h"
#include "global/hardware.h"
#include "global/observer.h"

struct frame
{
	int32 x;
	int32 y;
	uint32 width;
	uint32 height;
	ui_size<float32> width_desc;
	uint32 min_width;
	uint32 max_width;
	ui_size<float32> height_desc;
	uint32 min_height;
	uint32 max_height;
	ui_size<float32> margin_left;
	ui_size<float32> margin_bottom;
	ui_size<float32> margin_right;
	ui_size<float32> margin_top;
	ui_size<float32> padding_left;
	ui_size<float32> padding_bottom;
	ui_size<float32> padding_right;
	ui_size<float32> padding_top;
	bool visible;
	bool enabled;
	bool focusable;
	frame_background normal_background;
	frame_background hovered_background;
	frame_background focused_background;
	function<bool(vector<int32, 2> point)> hit_test;
	function<bool(vector<int32, 2> point)> content_hit_test;
	function<void(array<frame *> *frames)> subframes;
	function<vector<uint32, 2>(uint32 viewport_width, uint32 viewport_height)> content_size;
	function<void(graphics_displayer *gd, bitmap *bmp)> render;
	observer<> mouse_click;
	bool hook_mouse_click;
	bool return_mouse_click;
	observer<> mouse_release;
	bool hook_mouse_release;
	bool return_mouse_release;
	observer<> mouse_move;
	bool hook_mouse_move;
	bool return_mouse_move;
	observer<> mouse_wheel_rotate;
	bool hook_mouse_wheel_rotate;
	bool return_mouse_wheel_rotate;
	observer<> start_hover;
	observer<> end_hover;
	observer<> focus_receive;
	observer<> focus_loss;
	observer<> key_press;
	observer<> key_release;
	observer<> char_input;
	rectangle<int32> viewport();
	rectangle<int32> content_viewport();
	vector<uint32, 2> content_size_to_frame_size(uint32 content_width, uint32 content_height);
	vector<uint32, 2> calculate_frame_size(uint32 viewport_width, uint32 viewport_height);
	void render_background(graphics_displayer *gd, bitmap *bmp);
	void subframe_tree(array<frame *> *frames);
	bool is_parent_of(frame *fm);
	bool is_predecessor_of(frame *fm);

	frame();
};
