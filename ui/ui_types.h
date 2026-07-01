#pragma once
#include "graphics/graphics.h"

enum struct ui_size_type
{
	absolute,
	relative,
	autosize
};

template<typename value_type> struct ui_size
{
	ui_size_type type;
	value_type value;

	ui_size() {}

	ui_size(ui_size_type type, value_type value)
		: type(type), value(value) {}

	value_type resolve(value_type relative_size)
	{
		if(type == ui_size_type::relative)
			return value * relative_size;
		else return value;
	}
};

ui_size<float32> operator""uiabs(long double value);
ui_size<float32> operator""uirel(long double value);
ui_size<float32> operator""uiauto(long double value);

enum struct horizontal_align
{
	left,
	center,
	right
};

enum struct vertical_align
{
	top,
	center,
	bottom
};

enum struct frame_background_shape
{
	none,
	rectangle,
	rounded_rectangle
};

struct frame_background
{
	frame_background_shape shape;
	float32 rounded_rectangle_rx;
	float32 rounded_rectangle_ry;
	brush_switcher shape_brush;
	float32 outline_width;
	brush_switcher outline_brush;

	frame_background() {}
	frame_background(const frame_background &value);
	frame_background(frame_background &&value);
};
