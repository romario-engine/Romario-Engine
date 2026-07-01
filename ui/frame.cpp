#include "ui/frame.h"
#include "ui/ui.h"

frame::frame()
{
	width_desc = 200.0uiauto;
	min_width = 0;
	max_width = 1000000000;
	height_desc = 200.0uiauto;
	min_height = 0;
	max_height = 1000000000;
	margin_left = 0.0uiabs;
	margin_bottom = 0.0uiabs;
	margin_right = 0.0uiabs;
	margin_top = 0.0uiabs;
	padding_left =  0.0uiabs;
	padding_bottom = 0.0uiabs;
	padding_right = 0.0uiabs;
	padding_top = 0.0uiabs;
	visible = true;
	enabled = true;
	focusable = false;
	normal_background.shape = frame_background_shape::none;
	normal_background.outline_width = 0.0f;
	hovered_background.shape = frame_background_shape::none;
	hovered_background.outline_width = 0.0f;
	focused_background.shape = frame_background_shape::none;
	focused_background.outline_width = 0.0f;
	hit_test.assign_lambda([this] (vector<int32, 2> point) -> bool
		{
			return viewport().hit_test(point);
		});
	content_hit_test.assign_lambda([this] (vector<int32, 2> point) -> bool
		{
			rectangle<int32> content_rect = this->content_viewport();
			return content_rect.hit_test(point);
		});
	subframes.assign_addr(emit_empty_function<void(array<frame *> *frames)>());
	content_size.assign_addr(emit_empty_function<vector<uint32, 2>(uint32 viewport_width, uint32 viewport_height)>());
	render.assign_addr(emit_empty_function<void(graphics_displayer *gd, bitmap *bmp)>());
	hook_mouse_click = false;
	return_mouse_click = false;
	hook_mouse_release = false;
	return_mouse_release = false;
	hook_mouse_move = false;
	return_mouse_move = false;
	hook_mouse_wheel_rotate = false;
	return_mouse_wheel_rotate = false;
}

rectangle<int32> frame::viewport()
{
	rectangle<int32> rect;
	rect.position.x = x + int32(margin_left.resolve(float32(width)));
	rect.position.y = y + int32(margin_bottom.resolve(float32(height)));
	rect.extent.x = int32(width) - (rect.position.x - x)
		- int32(margin_right.resolve(float32(width)));
	rect.extent.y = int32(height) - (rect.position.y - y)
		- int32(margin_top.resolve(float32(height)));
	return rect;
}

rectangle<int32> frame::content_viewport()
{
	rectangle<int32> rect;
	rect.position.x = x
		+ int32(margin_left.resolve(float32(width)))
		+ int32(padding_left.resolve(float32(width)));
	rect.position.y = y
		+ int32(margin_bottom.resolve(float32(height)))
		+ int32(padding_bottom.resolve(float32(height)));
	rect.extent.x = width - (rect.position.x - x)
		- int32(margin_right.resolve(float32(width)))
		- int32(padding_right.resolve(float32(width)));
	rect.extent.y = height - (rect.position.y - y)
		- int32(margin_top.resolve(float32(height)))
		- int32(padding_top.resolve(float32(height)));
	return rect;
}

vector<uint32, 2> frame::content_size_to_frame_size(uint32 content_width, uint32 content_height)
{
	vector<uint32, 2> size(content_width, content_height);
	float32 denominator = 1.0f;
	if(margin_left.type == ui_size_type::relative)
		denominator -= margin_left.value;
	else size.x += margin_left.value;
	if(padding_left.type == ui_size_type::relative)
		denominator -= padding_left.value;
	else size.x += padding_left.value;
	if(margin_right.type == ui_size_type::relative)
		denominator -= margin_right.value;
	else size.x += margin_right.value;
	if(padding_right.type == ui_size_type::relative)
		denominator -= padding_right.value;
	else size.x += padding_right.value;
	size.x = float32(size.x) / denominator;
	denominator = 1.0f;
	if(margin_bottom.type == ui_size_type::relative)
		denominator -= margin_bottom.value;
	else size.y += margin_bottom.value;
	if(padding_bottom.type == ui_size_type::relative)
		denominator -= padding_bottom.value;
	else size.y += padding_bottom.value;
	if(margin_top.type == ui_size_type::relative)
		denominator -= margin_top.value;
	else size.y += margin_top.value;
	if(padding_top.type == ui_size_type::relative)
		denominator -= padding_top.value;
	else size.y += padding_top.value;
	size.y = float32(size.y) / denominator;
	return size;
}

vector<uint32, 2> frame::calculate_frame_size(uint32 parent_width, uint32 parent_height)
{
	vector<uint32, 2> size1, size2;
	size1.x = uint32(width_desc.resolve(float32(parent_width)));
	size1.x = min(max(min_width, size1.x), max_width);
	size1.y = uint32(height_desc.resolve(float32(parent_height)));
	size1.y = min(max(min_height, size1.y), max_height);
	if(width_desc.type == ui_size_type::autosize
		|| height_desc.type == ui_size_type::autosize)
	{
		if(width_desc.type == ui_size_type::autosize)
			size2.x = uint32(width_desc.value);
		else size2.x = size1.x;
		size2.x -= uint32(margin_left.resolve(float32(size2.x)))
			+ uint32(padding_left.resolve(float32(size2.x)))
			+ uint32(margin_right.resolve(float32(size2.x)))
			+ uint32(padding_right.resolve(float32(size2.x)));
		if(height_desc.type == ui_size_type::autosize)
			size2.y = uint32(height_desc.value);
		else size2.y = size1.y;
		size2.y -= uint32(margin_bottom.resolve(float32(size2.y)))
			+ uint32(padding_bottom.resolve(float32(size2.y)))
			+ uint32(margin_top.resolve(float32(size2.y)))
			+ uint32(padding_top.resolve(float32(size2.y)));
		size2 = content_size(size2.x, size2.y);
		size2 = content_size_to_frame_size(size2.x, size2.y);
		if(width_desc.type == ui_size_type::autosize)
		{
			size1.x = size2.x;
			size1.x = min(max(min_width, size1.x), max_width);
		}
		if(height_desc.type == ui_size_type::autosize)
		{
			size1.y = size2.y;
			size1.y = min(max(min_height, size1.y), max_height);
		}
	}
	return size1;
}

void frame::render_background(graphics_displayer *gd, bitmap *bmp)
{
	frame_background *bg;
	if(ui()->focused_frame == this)
		bg = &focused_background;
	else if(ui()->hovered_frame == this)
		bg = &hovered_background;
	else bg = &normal_background;
	if(bg->shape == frame_background_shape::none) return;
	geometry_path path;
	rectangle rect = viewport();
	if(bg->shape == frame_background_shape::rectangle)
		path.push_rectangle(rect);
	else if(bg->shape == frame_background_shape::rounded_rectangle)
		path.push_rounded_rectangle(
			rounded_rectangle<float32>(rect, bg->rounded_rectangle_rx, bg->rounded_rectangle_ry));
	if(!(bg->shape_brush.type == brush_type::solid && bg->shape_brush.color_value.a == 0))
	{
		gd->rasterization = rasterization_mode::fill;
		copy(bg->shape_brush, &gd->brush);
		gd->render(path, bmp);
	}
	if(!(bg->outline_width == 0.0f || bg->outline_brush.type == brush_type::solid && bg->outline_brush.color_value.a == 0))
	{
		gd->rasterization = rasterization_mode::outline;
		gd->line_width = bg->outline_width;
		copy(bg->outline_brush, &gd->brush);
		gd->render(path, bmp);
	}
}

void frame::subframe_tree(array<frame *> *frames)
{
	uint64 idx_begin = frames->size, idx_end;
	subframes(frames);
	idx_end = frames->size;
	while(idx_begin != idx_end)
	{
		(*frames)[idx_begin]->subframe_tree(frames);
		idx_begin++;
	}
}

bool frame::is_parent_of(frame *fm)
{
	array<frame *> frames;
	subframes(&frames);
	for(uint64 i = 0; i < frames.size; i++)
		if(frames[i] == fm) return true;
	return false;
}

bool frame::is_predecessor_of(frame *fm)
{
	array<frame *> frames;
	subframe_tree(&frames);
	for(uint64 i = 0; i < frames.size; i++)
		if(frames[i] == fm) return true;
	return false;
}
