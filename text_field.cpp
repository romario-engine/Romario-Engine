#include "text_field.h"
#include "ui.h"
#include "os_api.h"

text_field::text_field(bool is_editable, bool is_multiline) : scroll(scroll_bar_orientation::vertical)
{
	fm.subframes.assign_lambda([this] (array<frame *> *frames) -> void
		{
			for(uint64 i = 0; i < this->tl.glyphs.size; i++)
				if(this->tl.glyphs[i].type == glyph_type::frame)
					frames->push_back(this->tl.glyphs[i].fm);
			frames->push_back(&this->scroll.fm);
		});
	fm.content_size.assign_lambda([this] (uint32 viewport_width, uint32 viewport_height) -> vector<uint32, 2>
		{
			if(ui()->focused_frame == &this->fm && editable)
			{
				ui()->caret_frame.height_desc = ui_size<float32>(ui_size_type::absolute, font_size);
				this->tl.insert_frame(this->position, &ui()->caret_frame);
			}
			uint32 tl_width = this->tl.width;
			this->tl.width = viewport_width;
			vector<int32, 2> size = this->tl.content_size();
			this->tl.width = tl_width;
			for(uint64 i = 0; i < this->tl.glyphs.size; i++)
				if(this->tl.glyphs[i].type == glyph_type::frame && this->tl.glyphs[i].fm == &ui()->caret_frame)
					this->tl.glyphs.remove(i--);
			return vector<uint32, 2>(uint32(size.x), uint32(size.y));
				});
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			this->fm.render_background(gd, bmp);
			rectangle content_viewport = this->fm.content_viewport();
			if(!this->fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0) return;
			this->tl.width = uint32(content_viewport.extent.x);
			this->tl.height = uint32(content_viewport.extent.y);
			if(this->tl.multiline)
			{
				this->scroll.content_size = uint32(this->tl.content_size().y);
				this->scroll.viewport_size = uint32(content_viewport.extent.y);
				if(this->scroll.fm.visible && this->scroll.content_size > this->scroll.viewport_size)
				{
					this->scroll.fm.width = uint32(this->scroll.fm.width_desc.value);
					this->scroll.fm.height = uint32(content_viewport.extent.y);
					this->scroll.fm.x = content_viewport.position.x + content_viewport.extent.x - this->scroll.fm.width;
					this->scroll.fm.y = content_viewport.position.y;
					content_viewport.extent.x -= this->scroll.fm.width;
					if(content_viewport.extent.x <= 0) return;
					this->tl.width = uint32(content_viewport.extent.x);
					this->scroll.content_size = uint32(this->tl.content_size().y);
					this->scroll.fm.render(gd, bmp);
				}
				else
				{
					this->scroll.fm.width = 0;
					this->scroll.fm.height = 0;
				}
			}
			else
			{
				this->scroll.content_size = uint32(this->tl.content_size().x);
				this->scroll.viewport_size = uint32(content_viewport.extent.x);
				this->scroll.viewport_offset =
					min(this->scroll.viewport_offset, this->scroll.content_size - this->scroll.viewport_size);
				this->scroll.fm.width = 0;
				this->scroll.fm.height = 0;
			}
			gd->push_scissor(content_viewport);
			uint64 begin = this->position, end = this->select_position;
			if(end < begin) swap(&begin, &end);
			for(uint64 i = 0; i < this->tl.glyphs.size; i++)
				if(i >= begin && i < end) this->tl.glyphs[i].background_color = this->background_selecting_color;
				else this->tl.glyphs[i].background_color = color(0, 0, 0, 0);
			vector<int32, 2> text_point;
			if(this->tl.multiline)
				text_point = vector<int32, 2>(
					content_viewport.position.x,
					content_viewport.position.y
						+ content_viewport.extent.y
						- int32(this->tl.height)
						+ int32(this->scroll.viewport_offset));
			else text_point = vector<int32, 2>(
				content_viewport.position.x - int32(this->scroll.viewport_offset),
				content_viewport.position.y
					+ content_viewport.extent.y
					- int32(this->tl.height));
			if(ui()->focused_frame == &this->fm && editable)
			{
				ui()->caret_frame.height_desc = ui_size<float32>(ui_size_type::absolute, this->font_size);
				this->tl.insert_frame(this->position, &ui()->caret_frame);
			}
			this->tl.render(gd, text_point, bmp);
			for(uint64 i = 0; i < this->tl.glyphs.size; i++)
				if(this->tl.glyphs[i].type == glyph_type::frame && this->tl.glyphs[i].fm == &ui()->caret_frame)
					this->tl.glyphs.remove(i--);
			gd->pop_scissor();
		});
	fm.mouse_click.callbacks.move_back(function<void()>([this] () -> void
		{
			this->update_position_by_mouse();
			this->select_position = this->position;
			this->selecting = true;
		}));
	fm.mouse_move.callbacks.move_back(function<void()>([this] () -> void
		{
			if(this->selecting)
			{
				if(!mouse()->left_pressed)
					this->selecting = false;
				else this->update_position_by_mouse();
			}
		}));
	fm.focus_receive.callbacks.move_back(function<void()>([this] () -> void
		{
			if(!this->editable) return;
			ui()->caret_visible = true;
			ui()->caret_timer.reset();
			ui()->caret_timer.run();
		}));
	fm.focus_loss.callbacks.move_back(function<void()>([this] () -> void
		{
			ui()->caret_timer.reset();
			this->position = 0;
			this->select_position = 0;
			this->selecting = false;
		}));
	fm.mouse_wheel_rotate.callbacks.move_back(function<void()>([this] () -> void
		{
			if(mouse()->roll_direction == mouse_wheel_roll_direction::forward)
				this->scroll.shift(-this->scroll.roll_delta);
			else this->scroll.shift(this->scroll.roll_delta);
		}));
	fm.key_press.callbacks.move_back(function<void()>([this] () -> void
		{
			if(keyboard()->pressed_count == 1)
			{
				if(keyboard()->key_pressed[uint8(key_code::left)] && position != 0)
				{
					this->position--;
					this->select_position = this->position;
					this->scroll_to_caret();
				}
				else if(keyboard()->key_pressed[uint8(key_code::right)] && this->position != this->tl.glyphs.size)
				{
					this->position++;
					this->select_position = this->position;
					this->scroll_to_caret();
				}
				else if(keyboard()->key_pressed[uint8(key_code::down)] && this->tl.multiline && this->tl.glyphs.size != 0)
				{
					vector<int32, 2> point;
					int32 line_height;
					this->tl.hit_test_position(this->position, &point, &line_height);
					point.y--;
					this->tl.hit_test_point(point, &this->position);
					this->select_position = this->position;
					this->scroll_to_caret();
				}
				else if(keyboard()->key_pressed[uint8(key_code::up)] && this->tl.multiline && this->tl.glyphs.size != 0)
				{
					vector<int32, 2> point;
					int32 line_height;
					this->tl.hit_test_position(this->position, &point, &line_height);
					point.y += line_height + 1;
					this->tl.hit_test_point(point, &this->position);
					this->select_position = this->position;
					this->scroll_to_caret();
				}
			}
			else if(keyboard()->pressed_count == 2 && keyboard()->key_pressed[uint8(key_code::ctrl)])
			{
				if(keyboard()->key_pressed[uint8(key_code::c)] && this->position != this->select_position)
				{
					uint64 begin = this->position, end = this->select_position;
					if(end < begin) swap(&begin, &end);
					u16string str;
					while(begin < end)
					{
						str.push_back(char16(this->tl.glyphs[begin].data->code));
						begin++;
					}
					os_copy_text_to_clipboard(str);
				}
				else if(keyboard()->key_pressed[uint8(key_code::v)] && this->editable)
				{
					u16string str;
					os_copy_text_from_clipboard(&str);
					this->insert_text(str);
					this->scroll_to_caret();
				}
				else if(keyboard()->key_pressed[uint8(key_code::x)] && this->editable)
				{
					uint64 begin = this->position, end = this->select_position;
					if(end < begin) swap(&begin, &end);
					u16string str;
					while(begin < end)
					{
						str.push_back(char16(this->tl.glyphs[begin].data->code));
						begin++;
					}
					os_copy_text_to_clipboard(str);
					this->remove();
					this->scroll_to_caret();
				}
				else if(keyboard()->key_pressed[uint8(key_code::a)])
				{
					this->select(this->tl.glyphs.size, 0);
					this->scroll_to_caret();
				}
			}
		}));
	fm.char_input.callbacks.move_back(function<void()>([this] () -> void
		{
			if(!this->editable
				|| keyboard()->key_pressed[uint8(key_code::ctrl)]
				|| keyboard()->key_pressed[uint8(key_code::alt)])
				return;
			if(keyboard()->char_code == char32(key_code::backspace))
			{
				if(this->position == this->select_position
					&& this->position != 0
					&& this->tl.glyphs.size != 0)
					this->select(this->position - 1, this->position);
				this->remove();
				this->scroll_to_caret();
			}
			else if(keyboard()->char_code == char32(key_code::enter)
				&& !this->tl.multiline
				&& ui()->focused_frame == &this->fm)
				ui()->focus_frame(nullptr);
			else if(keyboard()->char_code == char32(key_code::enter)
				|| keyboard()->char_code == char32(key_code::tab)
				|| keyboard()->char_code >= U' ')
			{
				u16string str;
				if(keyboard()->char_code == char32(key_code::enter))
					str.push_back(u'\n');
				else str.push_back(keyboard()->char_code);
				this->insert_text(str);
				this->scroll_to_caret();
			}
		}));
	setup(is_editable, is_multiline);
	scroll.roll_delta = 50;
	position = 0;
	select_position = 0;
	selecting = false;
	background_selecting_color = color(0, 100, 200, 255);
	font << u"cambria";
	font_size = 20;
	italic = false;
	weight = 400;
	underlined = false;
	strikedthrough = false;
	text_color = color(0, 0, 0, 255);
}

void text_field::setup(bool is_editable, bool is_multiline)
{
	if(is_editable)
	{
		editable = true;
		fm.focusable = true;
		fm.normal_background.shape = frame_background_shape::rounded_rectangle;
		fm.normal_background.rounded_rectangle_rx = 3.0f;
		fm.normal_background.rounded_rectangle_ry = 3.0f;
		fm.normal_background.shape_brush.switch_solid_color(color(180, 180, 180, 255));
		fm.normal_background.outline_width = 2.0f;
		fm.normal_background.outline_brush.switch_solid_color(color(0, 0, 0, 255));
		copy(fm.normal_background, &fm.hovered_background);
		fm.hovered_background.shape_brush.switch_solid_color(color(220, 220, 220, 255));
		copy(fm.hovered_background, &fm.focused_background);
	}
	else
	{
		editable = false;
		fm.focusable = false;
		fm.normal_background.shape = frame_background_shape::none;
		fm.hovered_background.shape = frame_background_shape::none;
		fm.focused_background.shape = frame_background_shape::none;
	}
	tl.multiline = is_multiline;
}

void text_field::update_position_by_mouse()
{
	rectangle<int32> content_viewport = fm.content_viewport();
	if(tl.multiline)
		tl.hit_test_point(
			vector<int32, 2>(
				mouse()->position.x - content_viewport.position.x,
				mouse()->position.y - (content_viewport.position.y + int32(scroll.viewport_offset))),
			&position);
	else tl.hit_test_point(
			vector<int32, 2>(
				mouse()->position.x - (content_viewport.position.x - int32(scroll.viewport_offset)),
				mouse()->position.y - content_viewport.position.y),
			&position);
}

void text_field::move(uint64 idx)
{
	position = idx;
	select_position = idx;
}

void text_field::select(uint64 begin, uint64 end)
{
	position = end;
	select_position = begin;
}

void text_field::insert_text(const u16string &text)
{
	remove();
	tl.insert_text(position, text, font, font_size, italic, weight, underlined, strikedthrough, text_color);
	position += text.size;
	select_position = position;
}

void text_field::insert_frame(frame *frame_addr)
{
	remove();
	tl.insert_frame(position, frame_addr);
	position++;
	select_position = position;
}

void text_field::remove()
{
	if(select_position < position)
		swap(&position, &select_position);
	tl.glyphs.remove_range(position, select_position);
	select_position = position;
}

void text_field::scroll_to_caret()
{
	if(tl.glyphs.size == 0) return;
	rectangle<int32> content_viewport = fm.content_viewport();
	vector<int32, 2> point;
	int32 line_height;
	tl.hit_test_position(position, &point, &line_height);
	if(tl.multiline)
	{
		point.y += content_viewport.extent.y + int32(scroll.viewport_offset) - int32(tl.height);
		if(point.y < 0)
			scroll.shift(-point.y);
		else if(point.y + line_height >= content_viewport.extent.y)
			scroll.shift(-(point.y + line_height - content_viewport.extent.y));
	}
	else
	{
		point.x -= int32(scroll.viewport_offset);
		if(point.x < 0)
			scroll.shift(point.x);
		else if(point.x >= content_viewport.extent.x)
			scroll.shift(point.x - content_viewport.extent.x);
	}
}
