#include "flow_layout.h"

struct flow_layout_line_metrics
{
	uint64 frame_count;
	int32 linespace;
	int32 size1;
	int32 size2;
	int32 size3;

	flow_layout_line_metrics()
		: frame_count(0), linespace(0),
		size1(0), size2(0), size3(0) {}
};

struct flow_layout_metrics
{
	flow_layout *fl;
	int32 viewport_width;
	int32 viewport_height;
	int32 content_width;
	int32 content_height;
	array<flow_layout_line_metrics> line_metrics;

	flow_layout_metrics(flow_layout *fl, int32 viewport_width, int32 viewport_height)
		: fl(fl), viewport_width(viewport_width), viewport_height(viewport_height),
		content_width(0), content_height(0) {}
};

void flow_layout_evaluate_metrics(flow_layout_metrics *metrics)
{
	flow_layout_line_metrics line;
	horizontal_align last_halign = horizontal_align::left;
	vertical_align last_valign = vertical_align::top;
	flow_layout_frame *flf;
	uint64 end_idx = 0;
	vector<uint32, 2> size;
	if(metrics->fl->direction == flow_axis::x)
	{
		for(uint64 i = 0; i < metrics->fl->frames.size; i++)
		{
			flf = &metrics->fl->frames[i];
			size = flf->fm->calculate_frame_size(metrics->viewport_width, metrics->viewport_height);
			if(metrics->fl->multiline
				&& i != end_idx
				&& (flf->line_break
					|| uint32(flf->halign) < uint32(last_halign)
					|| line.size1 + line.size2 + line.size3 + (int32)size.x > metrics->viewport_width))
			{
				line.frame_count = i - end_idx;
				metrics->content_width = max(metrics->content_width, line.size1 + line.size2 + line.size3);
				metrics->content_height += line.linespace;
				metrics->line_metrics.push_back(line);
				end_idx = i;
				line = flow_layout_line_metrics();
			}
			last_halign = flf->halign;
			if(flf->halign == horizontal_align::left) line.size1 += int32(size.x);
			else if(flf->halign == horizontal_align::center) line.size2 += int32(size.x);
			else line.size3 += int32(size.x);
			line.linespace = max(line.linespace, int32(size.y));
		}
		if(end_idx != metrics->fl->frames.size)
		{
			line.frame_count = metrics->fl->frames.size - end_idx;
			metrics->content_width = max(metrics->content_width, line.size1 + line.size2 + line.size3);
			metrics->content_height += line.linespace;
			metrics->line_metrics.push_back(line);
		}
	}
	else
	{
		for(uint64 i = 0; i < metrics->fl->frames.size; i++)
		{
			flf = &metrics->fl->frames[i];
			size = flf->fm->calculate_frame_size(metrics->viewport_width, metrics->viewport_height);
			if(metrics->fl->multiline
				&& i != end_idx
				&& (flf->line_break
					|| uint32(flf->valign) < uint32(last_valign)
					|| line.size1 + line.size2 + line.size3 + (int32)size.y > metrics->viewport_height))
			{
				line.frame_count = i - end_idx;
				metrics->content_height = max(metrics->content_height, line.size1 + line.size2 + line.size3);
				metrics->content_width += line.linespace;
				metrics->line_metrics.push_back(line);
				end_idx = i;
				line = flow_layout_line_metrics();
			}
			last_valign = flf->valign;
			if(flf->valign == vertical_align::bottom) line.size1 += int32(size.y);
			else if(flf->valign == vertical_align::center) line.size2 += int32(size.y);
			else line.size3 += int32(size.y);
			line.linespace = max(line.linespace, int32(size.x));
		}
		if(end_idx != metrics->fl->frames.size)
		{
			line.frame_count = metrics->fl->frames.size - end_idx;
			metrics->content_height = max(metrics->content_height, line.size1 + line.size2 + line.size3);
			metrics->content_width += line.linespace;
			metrics->line_metrics.push_back(line);
		}
	}
}

flow_layout::flow_layout() : xscroll(scroll_bar_orientation::horizontal), yscroll(scroll_bar_orientation::vertical)
{
	direction = flow_axis::x;
	offset = flow_line_offset::right;
	multiline = true;
	fm.subframes.assign_lambda([this] (array<frame *> *frames_array) -> void
		{
			frames_array->push_back(&this->xscroll.fm);
			frames_array->push_back(&this->yscroll.fm);
			for(uint64 i = 0; i < this->frames.size; i++)
				frames_array->push_back(this->frames[i].fm);
		});
	fm.content_size.assign_lambda([this] (uint32 viewport_width, uint32 viewport_height) -> vector<uint32, 2>
		{
			flow_layout_metrics metrics(this, viewport_width, viewport_height);
			flow_layout_evaluate_metrics(&metrics);
			if(metrics.content_width > int32(viewport_width))
				metrics.content_height += int32(this->xscroll.fm.height_desc.value);
			if(metrics.content_height > int32(viewport_height))
				metrics.content_width += int32(this->yscroll.fm.width_desc.value);
			return vector<uint32, 2>(metrics.content_width, metrics.content_height);
		});
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			fm.render_background(gd, bmp);
			rectangle<int32> content_viewport = fm.content_viewport();
			if(!fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0) return;
			update_layout();
			gd->push_scissor(content_viewport);
			if(this->xscroll.content_size > this->xscroll.viewport_size)
				this->xscroll.viewport_offset
					= min(this->xscroll.viewport_offset, this->xscroll.content_size - this->xscroll.viewport_size);
			else this->xscroll.viewport_offset = 0;
			if(this->yscroll.content_size > this->yscroll.viewport_size)
				this->yscroll.viewport_offset
					= min(this->yscroll.viewport_offset, this->yscroll.content_size - this->yscroll.viewport_size);
			else this->yscroll.viewport_offset = 0;
			for(uint64 i = 0; i < this->frames.size; i++)
			{
				this->frames[i].fm->x -= int32(this->xscroll.viewport_offset);
				this->frames[i].fm->y += int32(this->yscroll.viewport_offset);
				if(this->frames[i].fm->x < content_viewport.position.x + content_viewport.extent.x
					&& this->frames[i].fm->x + int32(this->frames[i].fm->width) >= content_viewport.position.x
					&& this->frames[i].fm->y < content_viewport.position.y + content_viewport.extent.y
					&& this->frames[i].fm->y + int32(this->frames[i].fm->height) >= content_viewport.position.y)
					this->frames[i].fm->render(gd, bmp);
			}
			gd->pop_scissor();
			this->xscroll.fm.render(gd, bmp);
			this->yscroll.fm.render(gd, bmp);
		});
	fm.mouse_wheel_rotate.callbacks.move_back(function<void()>([this] () -> void
		{
			if(this->yscroll.fm.visible)
			{
				if(mouse()->roll_direction == mouse_wheel_roll_direction::forward)
					this->yscroll.shift(-this->yscroll.roll_delta);
				else this->yscroll.shift(this->yscroll.roll_delta);
			}
			else
			{
				if(mouse()->roll_direction == mouse_wheel_roll_direction::forward)
					this->xscroll.shift(-this->xscroll.roll_delta);
				else this->xscroll.shift(this->xscroll.roll_delta);
			}
		}));
}

void flow_layout::update_layout()
{
	rectangle<int32> viewport = fm.viewport();
	rectangle<int32> content_viewport = fm.content_viewport();
	flow_layout_metrics metrics(this, content_viewport.extent.x, content_viewport.extent.y);
	flow_layout_evaluate_metrics(&metrics);
	bool reevaluate = false;
	xscroll.fm.height = uint32(xscroll.fm.height_desc.value);
	yscroll.fm.width = uint32(yscroll.fm.width_desc.value);
	if(metrics.content_width > metrics.viewport_width)
	{
		reevaluate = true;
		metrics.viewport_height -= xscroll.fm.height;
		xscroll.fm.visible = true;
	}
	else xscroll.fm.visible = false;
	if(metrics.content_height > metrics.viewport_height)
	{
		reevaluate = true;
		metrics.viewport_width -= yscroll.fm.width;
		yscroll.fm.visible = true;
	}
	else yscroll.fm.visible = false;
	if(reevaluate)
	{
		metrics.content_width = 0;
		metrics.content_height = 0;
		metrics.line_metrics.clear();
		flow_layout_evaluate_metrics(&metrics);
		xscroll.content_size = uint32(metrics.content_width);
		xscroll.viewport_size = uint32(metrics.viewport_width);
		yscroll.content_size = uint32(metrics.content_height);
		yscroll.viewport_size = uint32(metrics.viewport_height);
		xscroll.fm.width = uint32(content_viewport.extent.x);
		yscroll.fm.height = uint32(content_viewport.extent.y);
		if(xscroll.fm.visible && yscroll.fm.visible)
		{
			xscroll.fm.width -= yscroll.fm.width;
			yscroll.fm.height -= xscroll.fm.height;
		}
		xscroll.fm.x = content_viewport.position.x;
		xscroll.fm.y = content_viewport.position.y;
		yscroll.fm.x = content_viewport.position.x
			+ content_viewport.extent.x - int32(yscroll.fm.width);
		yscroll.fm.y = content_viewport.position.y
			+ content_viewport.extent.y - int32(yscroll.fm.height);
	}
	int32 offset1, offset2, offset3, fi = 0;
	vector<uint32, 2> size;
	vector<int32, 2> line_position;
	if(direction == flow_axis::x)
	{
		if(offset == flow_line_offset::right)
			line_position = vector<int32, 2>(
				content_viewport.position.x,
				content_viewport.position.y + content_viewport.extent.y);
		else line_position = vector<int32, 2>(
			content_viewport.position.x,
			content_viewport.position.y + content_viewport.extent.y - metrics.content_height);
		for(uint64 i = 0; i < metrics.line_metrics.size; i++)
		{
			if(offset == flow_line_offset::right)
				line_position.y -= metrics.line_metrics[i].linespace;
			offset1 = 0;
			offset2 = max(
				(metrics.viewport_width - metrics.line_metrics[i].size2) / 2,
				metrics.line_metrics[i].size1);
			if(metrics.line_metrics[i].size2 == 0)
				offset2 = metrics.line_metrics[i].size1;
			offset3 = max(
				metrics.viewport_width - metrics.line_metrics[i].size3,
				offset2 + metrics.line_metrics[i].size2);
			if(metrics.content_width <= metrics.viewport_width
				&& offset3 + metrics.line_metrics[i].size3 > metrics.viewport_width)
			{
				offset3 = metrics.viewport_width - metrics.line_metrics[i].size3;
				offset2 = offset3 - metrics.line_metrics[i].size2;
			}
			for(uint64 j = metrics.line_metrics[i].frame_count; j != 0; j--, fi++)
			{
				if(fm.height_desc.type == ui_size_type::autosize
					&& frames[fi].fm->height_desc.type == ui_size_type::relative)
					size = frames[fi].fm->calculate_frame_size(
						metrics.viewport_width,
						metrics.line_metrics[i].linespace);
				else size = frames[fi].fm->calculate_frame_size(
					metrics.viewport_width,
					metrics.viewport_height);
				frames[fi].fm->width = size.x;
				frames[fi].fm->height = size.y;
				if(frames[fi].halign == horizontal_align::left)
				{
					frames[fi].fm->x = line_position.x + offset1;
					offset1 += int32(frames[fi].fm->width);
				}
				else if(frames[fi].halign == horizontal_align::center)
				{
					frames[fi].fm->x = line_position.x + offset2;
					offset2 += int32(frames[fi].fm->width);
				}
				else
				{
					frames[fi].fm->x = line_position.x + offset3;
					offset3 += int32(frames[fi].fm->width);
				}
				if(frames[fi].valign == vertical_align::top)
					frames[fi].fm->y = line_position.y
						+ metrics.line_metrics[i].linespace
						- int32(frames[fi].fm->height);
				else if(frames[fi].valign == vertical_align::center)
					frames[fi].fm->y = line_position.y
						+ (metrics.line_metrics[i].linespace - int32(frames[fi].fm->height)) / 2;
				else frames[fi].fm->y = line_position.y;
			}
			if(offset == flow_line_offset::left)
				line_position.y += metrics.line_metrics[i].linespace;
		}
	}
	else
	{
		if(offset == flow_line_offset::left)
			line_position = vector<int32, 2>(
				content_viewport.position.x,
				content_viewport.position.y + content_viewport.extent.y - metrics.content_height);
		else line_position = vector<int32, 2>(
			content_viewport.position.x + content_viewport.extent.x,
			content_viewport.position.y + content_viewport.extent.y - metrics.content_height);
		for(uint64 i = 0; i < metrics.line_metrics.size; i++)
		{
			if(offset == flow_line_offset::right)
				line_position.x -= metrics.line_metrics[i].linespace;
			offset1 = 0;
			offset2 = max(
				(metrics.viewport_height - metrics.line_metrics[i].size2) / 2,
				metrics.line_metrics[i].size1);
			if(metrics.line_metrics[i].size2 == 0)
				offset2 = metrics.line_metrics[i].size1;
			offset3 = max(
				metrics.viewport_height - metrics.line_metrics[i].size3,
				offset2 + metrics.line_metrics[i].size2);
			if(metrics.content_height <= metrics.viewport_height
				&& offset3 + metrics.line_metrics[i].size3 > metrics.viewport_height)
			{
				offset3 = metrics.viewport_height - metrics.line_metrics[i].size3;
				offset2 = offset3 - metrics.line_metrics[i].size2;
			}
			for(uint64 j = metrics.line_metrics[i].frame_count; j != 0; j--, fi++)
			{
				if(fm.width_desc.type == ui_size_type::autosize
					&& frames[fi].fm->width_desc.type == ui_size_type::relative)
					size = frames[fi].fm->calculate_frame_size(
						metrics.line_metrics[i].linespace,
						metrics.viewport_height);
				else size = frames[fi].fm->calculate_frame_size(
					metrics.viewport_width,
					metrics.viewport_height);
				frames[fi].fm->width = size.x;
				frames[fi].fm->height = size.y;
				if(frames[fi].valign == vertical_align::bottom)
				{
					frames[fi].fm->y = line_position.y + offset1;
					offset1 += int32(frames[fi].fm->height);
				}
				else if(frames[fi].valign == vertical_align::center)
				{
					frames[fi].fm->y = line_position.y + offset2;
					offset2 += int32(frames[fi].fm->height);
				}
				else
				{
					frames[fi].fm->y = line_position.y + offset3;
					offset3 += int32(frames[fi].fm->height);
				}
				if(frames[fi].halign == horizontal_align::right)
					frames[fi].fm->x = line_position.x
						+ metrics.line_metrics[i].linespace
						- int32(frames[fi].fm->width);
				else if(frames[fi].halign == horizontal_align::center)
					frames[fi].fm->x = line_position.x
						+ (metrics.line_metrics[i].linespace - int32(frames[fi].fm->width)) / 2;
				else frames[fi].fm->x = line_position.x;
			}
			if(offset == flow_line_offset::left)
				line_position.x += metrics.line_metrics[i].linespace;
		}
	}
}
