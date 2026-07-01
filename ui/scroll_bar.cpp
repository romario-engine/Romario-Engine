#include "ui/scroll_bar.h"

scroll_bar *moving_slider = nullptr;

void slider_rect_mouse_move()
{
	if(moving_slider->orientation == scroll_bar_orientation::vertical)
		moving_slider->shift((mouse()->position.y - mouse()->prev_position.y)
			* int32(moving_slider->content_size) / int32(moving_slider->viewport_size));
	else moving_slider->shift((mouse()->position.x - mouse()->prev_position.x)
		* int32(moving_slider->content_size) / int32(moving_slider->viewport_size));
}

void slider_rect_mouse_release()
{
	moving_slider = nullptr;
	mouse()->mouse_move.callbacks.remove_element(slider_rect_mouse_move);
	mouse()->mouse_release.callbacks.remove_element(slider_rect_mouse_release);
}

scroll_bar::scroll_bar(scroll_bar_orientation type)
{
	fm.width_desc = 12.0uiabs;
	fm.height_desc = 12.0uiabs;
	fm.margin_left = 0.0uiabs;
	fm.margin_bottom = 0.0uiabs;
	fm.margin_right = 0.0uiabs;
	fm.margin_top = 0.0uiabs;
	fm.padding_left = 2.0uiabs;
	fm.padding_bottom = 2.0uiabs;
	fm.padding_right = 2.0uiabs;
	fm.padding_top = 2.0uiabs;
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			this->fm.render_background(gd, bmp);
			rectangle<int32> content_viewport = this->fm.content_viewport();
			if(!this->fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0
				|| this->viewport_size >= this->content_size) return;
			this->viewport_offset = min(this->viewport_offset, this->content_size - this->viewport_size);
			rectangle<int32> slider_rect = this->slider_rectangle(content_viewport);
			gd->brush.switch_solid_color(color(127, 127, 127, 255));
			gd->fill_rect(slider_rect, bmp);
		});
	fm.mouse_click.callbacks.move_back(function<void()>([this] () -> void
		{
			rectangle<int32> slider_rect = this->slider_rectangle(this->fm.content_viewport());
			if(slider_rect.hit_test(mouse()->position) && moving_slider == nullptr)
			{
				moving_slider = this;
				mouse()->mouse_move.callbacks.move_back(function<void()>(slider_rect_mouse_move));
				mouse()->mouse_release.callbacks.move_back(function<void()>(slider_rect_mouse_release));
			}
		}));
	fm.mouse_wheel_rotate.callbacks.move_back(function<void()>([this] () -> void
		{
			if(mouse()->roll_direction == mouse_wheel_roll_direction::forward)
				this->shift(-this->roll_delta);
			else this->shift(this->roll_delta);
		}));
	orientation = type;
	min_slider_size = 6;
	roll_delta = 50;
	content_size = 0;
	viewport_size = 0;
	viewport_offset = 0;
}

void scroll_bar::shift(int32 value)
{
	if(value < 0)
	{
		if(viewport_offset < -value)
			viewport_offset = 0;
		else viewport_offset += value;
	}
	else viewport_offset += value;
}

rectangle<int32> scroll_bar::slider_rectangle(rectangle<int32> viewport)
{
	int32 slider_size = (orientation == scroll_bar_orientation::vertical ? viewport.extent.y : viewport.extent.x)
		* viewport_size / content_size;
	slider_size = min(max(slider_size, min_slider_size), (orientation == scroll_bar_orientation::vertical ? viewport.extent.y : viewport.extent.x));
	int32 slider_offset = ((orientation == scroll_bar_orientation::vertical ? viewport.extent.y : viewport.extent.x) - slider_size)
		* viewport_offset / (content_size - viewport_size);
	rectangle<int32> slider_rect;
	if(orientation == scroll_bar_orientation::vertical)
	{
		slider_rect.position = vector<int32, 2>(
			viewport.position.x,
			viewport.position.y + viewport.extent.y - slider_offset - slider_size);
		slider_rect.extent = vector<int32, 2>(viewport.extent.x, slider_size);
	}
	else
	{
		slider_rect.position = vector<int32, 2>(
			viewport.position.x + slider_offset,
			viewport.position.y);
		slider_rect.extent = vector<int32, 2>(slider_size, viewport.extent.y);
	}
	return slider_rect;
}
