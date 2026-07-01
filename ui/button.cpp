#include "ui/button.h"
#include "ui/window.h"
#include "ui/ui.h"

button *pressed_button;

void pressed_button_mouse_release()
{
	if(mouse()->last_released == mouse_button::left
		&& pressed_button->fm.hit_test(mouse()->position))
		pressed_button->button_click();
	mouse()->mouse_release.callbacks.remove_element(pressed_button_mouse_release);
	pressed_button = nullptr;
}

button::button()
{
	button_click.assign_addr(emit_empty_function<void()>());
	fm.hook_mouse_move = true;
	fm.hook_mouse_click = true;
	fm.hook_mouse_release = true;
	fm.hook_mouse_wheel_rotate = true;
	fm.subframes.assign_lambda([this] (array<frame *> *frames) -> void
		{
			frames->push_back(&this->layout.fm);
		});
	fm.content_size.assign_lambda([this] (uint32 viewport_width, uint32 viewport_height) -> vector<uint32, 2>
		{
			return this->layout.fm.calculate_frame_size(viewport_width, viewport_height);
		});
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			rectangle<int32> content_viewport = this->fm.content_viewport();
			this->layout.fm.x = content_viewport.position.x;
			this->layout.fm.y = content_viewport.position.y;
			this->layout.fm.width = content_viewport.extent.x;
			this->layout.fm.height = content_viewport.extent.y;
			this->layout.fm.render(gd, bmp);
		});
	fm.mouse_click.callbacks.move_back(function<void()>([this] () -> void
		{
			if(mouse()->last_clicked == mouse_button::left)
			{
				pressed_button = this;
				mouse()->mouse_release.callbacks.push_back(pressed_button_mouse_release);
			}
		}));
}

void button::setup_text_button(text_field **tf)
{
	layout.frames.reset();
	*tf = new text_field(false, true);
	(*tf)->tl.halign = horizontal_align::center;
	(*tf)->tl.valign = vertical_align::center;
	layout.frames.push_back(flow_layout_frame(&(*tf)->fm));
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			rectangle viewport = this->fm.viewport(),
				content_viewport = this->fm.content_viewport();
			if(!this->fm.visible
				|| viewport.extent.x <= 0 || viewport.extent.y <= 0
				|| content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0)
				return;
			geometry_path path;
			rounded_rectangle<float32> rrect(
				rectangle<float32>(viewport),
				0.07f * float32(viewport.extent.x),
				0.07f * float32(viewport.extent.y));
			path.push_rounded_rectangle(rrect);
			gd->rasterization = rasterization_mode::fill;
			if(this->pressed())
				gd->brush.switch_solid_color(color(170, 170, 170, 255));
			else if(ui()->hovered_frame == &this->fm)
				gd->brush.switch_solid_color(color(185, 185, 185, 255));
			else gd->brush.switch_solid_color(color(210, 210, 210, 255));
			gd->render(path, bmp);
			this->layout.fm.x = content_viewport.position.x;
			this->layout.fm.y = content_viewport.position.y;
			this->layout.fm.width = content_viewport.extent.x;
			this->layout.fm.height = content_viewport.extent.y;
			this->layout.fm.render(gd, bmp);
			if(this->pressed() || ui()->hovered_frame == &this->fm)
			{
				gradient_stop stops[2];
				stops[0].offset = 0.0f;
				stops[0].value = color(140, 140, 140, 255);
				stops[1].offset = 1.0f;
				stops[1].value = color(140, 140, 140, 0);
				vector<float32, 2> position = vector<float32, 2>(mouse()->position);
				position.x = max(position.x, float32(viewport.position.x));
				position.x = min(position.x, float32(viewport.position.x + viewport.extent.x));
				position.y = max(position.y, float32(viewport.position.y));
				position.y = min(position.y, float32(viewport.position.y + viewport.extent.y));
				float32 r = float32(max(viewport.extent.x, viewport.extent.y) / 2);
				gd->brush.switch_radial_gradient(stops, array_size(stops), position, vector<float32, 2>(0.0f, 0.0f), r, r);
				gd->color_interpolation = color_interpolation_mode::linear;
				gd->render(path, bmp);
			}
			gd->rasterization = rasterization_mode::outline;
			gd->line_width = 2.0f;
			gd->brush.switch_solid_color(color(100, 100, 100, 255));
			gd->render(path, bmp);
		});
}

void button::setup_highlighted_button(color hovered_color, color pressed_color)
{
	layout.frames.reset();
	fm.render.assign_lambda([this, hovered_color, pressed_color] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			rectangle<int32> content_viewport = this->fm.content_viewport();
			if(!this->fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0) return;
			if(this->pressed())
			{
				gd->brush.switch_solid_color(pressed_color);
				gd->fill_rect(content_viewport, bmp);
			}
			else if(ui()->hovered_frame == &this->fm)
			{
				gd->brush.switch_solid_color(hovered_color);
				gd->fill_rect(content_viewport, bmp);
			}
			this->layout.fm.x = content_viewport.position.x;
			this->layout.fm.y = content_viewport.position.y;
			this->layout.fm.width = content_viewport.extent.x;
			this->layout.fm.height = content_viewport.extent.y;
			this->layout.fm.render(gd, bmp);
		});
}

void button::setup_marked_radio_button(radio_button_group *group, frame **mark, text_field **tf)
{
	layout.frames.reset();
	frame *mark_frame = new frame();
	mark_frame->width_desc = 18.0uiabs;
	mark_frame->height_desc = 18.0uiabs;
	mark_frame->padding_left = 3.0uiabs;
	mark_frame->padding_right = 3.0uiabs;
	mark_frame->padding_bottom = 3.0uiabs;
	mark_frame->padding_top = 3.0uiabs;
	mark_frame->render.assign_lambda([this, group] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			rectangle<int32> content_viewport = this->fm.content_viewport();
			if(!this->fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0) return;
			float32 size = min(content_viewport.extent.x, content_viewport.extent.y);
			geometry_path path;
			path.move(vector<float32, 2>(
				content_viewport.position.x + 0.5f * (content_viewport.extent.x + size),
				content_viewport.position.y + 0.5f * content_viewport.extent.y));
			path.push_elliptic_arc(vector<float32, 2>(
				content_viewport.position.x + 0.5f * (content_viewport.extent.x + size),
				content_viewport.position.y + 0.5f * content_viewport.extent.y),
				0.5f * size,
				0.5f * size,
				0.0f,
				2.0f * pi,
				0.0f);
			if(ui()->hovered_frame == &this->fm)
			{
				gd->rasterization = rasterization_mode::fill;
				gd->brush.switch_solid_color(color(120, 120, 120, 255));
				gd->render(path, bmp);
			}
			gd->rasterization = rasterization_mode::outline;
			gd->line_width = 3.0f;
			gd->brush.switch_solid_color(color(0, 0, 0, 255));
			gd->render(path, bmp);
			if(group->selected_frame == &this->fm)
			{
				path.data.clear();
				path.move(vector<float32, 2>(
					content_viewport.position.x + 0.5f * (content_viewport.extent.x + 0.8f * size),
					content_viewport.position.y + 0.5f * content_viewport.extent.y));
				path.push_elliptic_arc(vector<float32, 2>(
					content_viewport.position.x + 0.5f * (content_viewport.extent.x + 0.8f * size),
					content_viewport.position.y + 0.5f * content_viewport.extent.y),
					0.5f * 0.6f *  size,
					0.5f * 0.6f * size,
					0.0f,
					2.0f * pi,
					0.0f);
				gd->rasterization = rasterization_mode::fill;
				gd->brush.switch_solid_color(color(0, 0, 0, 255));
				gd->render(path, bmp);
			}
		});
	layout.frames.push_back(flow_layout_frame(mark_frame, horizontal_align::left, vertical_align::center));
	*mark = mark_frame;
	text_field *tf_frame = new text_field(false, true);
	tf_frame->tl.halign = horizontal_align::center;
	tf_frame->tl.valign = vertical_align::center;
	layout.frames.push_back(flow_layout_frame(&tf_frame->fm, horizontal_align::left, vertical_align::center));
	*tf = tf_frame;
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			rectangle<int32> content_viewport = this->fm.content_viewport();
			this->layout.fm.x = content_viewport.position.x;
			this->layout.fm.y = content_viewport.position.y;
			this->layout.fm.width = content_viewport.extent.x;
			this->layout.fm.height = content_viewport.extent.y;
			this->layout.fm.render(gd, bmp);
		});
}

void button::setup_highlighted_radio_button(radio_button_group *group, color hovered_color, color selected_color)
{
	layout.frames.reset();
	fm.render.assign_lambda([this, group, hovered_color, selected_color] (graphics_displayer *gd, bitmap *bmp) -> void
	{
		rectangle<int32> content_viewport = this->fm.content_viewport();
		if(!this->fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0) return;
		geometry_path path;
		path.push_rectangle(content_viewport);
		gd->rasterization = rasterization_mode::outline;
		gd->line_width = 2.0f;
		if(group->selected_frame == &this->fm)
		{
			gd->brush.switch_solid_color(selected_color);
			gd->render(path, bmp);
		}
		else if(ui()->hovered_frame == &this->fm)
		{
			gd->brush.switch_solid_color(hovered_color);
			gd->render(path, bmp);
		}
		this->layout.fm.x = content_viewport.position.x;
		this->layout.fm.y = content_viewport.position.y;
		this->layout.fm.width = content_viewport.extent.x;
		this->layout.fm.height = content_viewport.extent.y;
		this->layout.fm.render(gd, bmp);
	});
}

bool button::pressed()
{
	return pressed_button == this;
}

radio_button_group::radio_button_group()
{
	selected_frame = nullptr;
}
