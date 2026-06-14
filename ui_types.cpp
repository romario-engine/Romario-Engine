#include "ui_types.h"

ui_size<float32> operator""uiabs(long double value)
{
	return ui_size<float32>(ui_size_type::absolute, float32(value));
}

ui_size<float32> operator""uirel(long double value)
{
	return ui_size<float32>(ui_size_type::relative, float32(value));
}

ui_size<float32> operator""uiauto(long double value)
{
	return ui_size<float32>(ui_size_type::autosize, float32(value));
}

frame_background::frame_background(const frame_background &value)
{
	shape = value.shape;
	rounded_rectangle_rx = value.rounded_rectangle_rx;
	rounded_rectangle_ry = value.rounded_rectangle_ry;
	copy(value.shape_brush, &shape_brush);
	outline_width = value.outline_width;
	copy(value.outline_brush, &outline_brush);
}

frame_background::frame_background(frame_background &&value)
{
	shape = value.shape;
	rounded_rectangle_rx = value.rounded_rectangle_rx;
	rounded_rectangle_ry = value.rounded_rectangle_ry;
	move(value.shape_brush, &shape_brush);
	outline_width = value.outline_width;
	move(value.outline_brush, &outline_brush);
}
