#include "grid_layout.h"

struct grid_layout_metrics
{
	grid_layout *gl;
	int32 viewport_width;
	int32 viewport_height;
	int32 content_width;
	int32 content_height;
	array<uint32> rows_size;
	array<uint32> columns_size;

	grid_layout_metrics(grid_layout *gl, int32 viewport_width, int32 viewport_height)
		: gl(gl), viewport_width(viewport_width), viewport_height(viewport_height)
	{
		content_width = 0;
		content_height = 0;
		rows_size.insert_default(0, gl->rows_desc.size);
		for(uint64 i = 0; i < rows_size.size; i++)
			rows_size[i] = 0;
		columns_size.insert_default(0, gl->columns_desc.size);
		for(uint64 i = 0; i < columns_size.size; i++)
			columns_size[i] = 0;
	}
};

void grid_layout_evaluate_metrics(grid_layout_metrics *metrics)
{
	grid_layout_frame *glf;
	vector<uint32, 2> size;
	for(uint64 i = 0; i < metrics->rows_size.size; i++)
		if(metrics->gl->rows_desc[i].type != ui_size_type::autosize)
			metrics->rows_size[i] = uint32(metrics->gl->rows_desc[i].resolve(float32(metrics->viewport_height)));
	for(uint64 j = 0; j < metrics->columns_size.size; j++)
		if(metrics->gl->columns_desc[j].type != ui_size_type::autosize)
			metrics->columns_size[j] = uint32(metrics->gl->columns_desc[j].resolve(float32(metrics->viewport_width)));
	for(uint64 i = 0; i < metrics->gl->frames.rows; i++)
	{
		if(i == metrics->gl->growth_row) continue;
		for(uint64 j = 0; j < metrics->gl->frames.columns; j++)
		{
			if(j == metrics->gl->growth_column) continue;
			glf = &metrics->gl->frames.at(i, j);
			if(glf->fm == nullptr) continue;
			if(metrics->gl->rows_desc[i].type == ui_size_type::autosize
				|| metrics->gl->columns_desc[j].type == ui_size_type::autosize)
			{
				size.x = uint32(glf->fm->width_desc.resolve(float32(metrics->columns_size[j])));
				size.x = min(max(glf->fm->min_width, size.x), glf->fm->max_width);
				size.y = uint32(glf->fm->height_desc.resolve(float32(metrics->rows_size[i])));
				size.y = min(max(glf->fm->min_height, size.y), glf->fm->max_height);
				size = glf->fm->content_size(size.x, size.y);
				size = glf->fm->content_size_to_frame_size(size.x, size.y);
				if(glf->fm->width_desc.type == ui_size_type::absolute)
					size.x = uint32(glf->fm->width_desc.value);
				if(glf->fm->height_desc.type == ui_size_type::absolute)
					size.y = uint32(glf->fm->height_desc.value);
				size.x = min(max(glf->fm->min_width, size.x), glf->fm->max_width);
				size.y = min(max(glf->fm->min_height, size.y), glf->fm->max_height);
			}
			if(metrics->gl->rows_desc[i].type == ui_size_type::autosize)
				metrics->rows_size[i] = max(metrics->rows_size[i], size.y);
			if(metrics->gl->columns_desc[j].type == ui_size_type::autosize)
				metrics->columns_size[j] = max(metrics->columns_size[j], size.x);
		}
	}
	for(uint64 i = 0; i < metrics->rows_size.size; i++)
		if(i != metrics->gl->growth_row)
			metrics->content_height += metrics->rows_size[i];
	for(uint64 j = 0; j < metrics->columns_size.size; j++)
		if(j != metrics->gl->growth_column)
			metrics->content_width += metrics->columns_size[j];
	if(metrics->gl->growth_row != not_exists<uint64>)
	{
		if(metrics->content_height >= metrics->viewport_height)
			metrics->rows_size[metrics->gl->growth_row] = 0;
		else
		{
			metrics->rows_size[metrics->gl->growth_row] = uint32(metrics->viewport_height - metrics->content_height);
			metrics->content_height = uint32(metrics->viewport_height);
		}
	}
	if(metrics->gl->growth_column != not_exists<uint64>)
	{
		if(metrics->content_width >= metrics->viewport_width)
			metrics->columns_size[metrics->gl->growth_column] = 0;
		else
		{
			metrics->columns_size[metrics->gl->growth_column] = uint32(metrics->viewport_width - metrics->content_width);
			metrics->content_width = uint32(metrics->viewport_width);
		}
	}
}

grid_layout::grid_layout() : xscroll(scroll_bar_orientation::horizontal), yscroll(scroll_bar_orientation::vertical)
{
	growth_row = not_exists<uint64>;
	growth_column = not_exists<uint64>;
	fm.subframes.assign_lambda([this] (array<frame *> *frames_array) -> void
		{
			frames_array->push_back(&this->xscroll.fm);
			frames_array->push_back(&this->yscroll.fm);
			for(uint64 i = 0; i < this->frames.rows * this->frames.columns; i++)
				if(this->frames.addr[i].fm != nullptr)
					frames_array->push_back(this->frames.addr[i].fm);
				});
	fm.content_size.assign_lambda([this] (uint32 viewport_width, uint32 viewport_height) -> vector<uint32, 2>
		{
			grid_layout_metrics metrics(this, viewport_width, viewport_height);
			grid_layout_evaluate_metrics(&metrics);
			if(metrics.content_width > int32(viewport_width))
				metrics.content_height += int32(this->xscroll.fm.height_desc.value);
			if(metrics.content_height > int32(viewport_height))
				metrics.content_width += int32(this->yscroll.fm.width_desc.value);
			return vector<uint32, 2>(metrics.content_width, metrics.content_height);
		});
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			this->fm.render_background(gd, bmp);
			rectangle<int32> content_viewport = this->fm.content_viewport();
			if(!this->fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0) return;
			this->update_layout();
			gd->push_scissor(content_viewport);
			if(this->xscroll.content_size > this->xscroll.viewport_size)
				this->xscroll.viewport_offset
					= min(this->xscroll.viewport_offset, this->xscroll.content_size - this->xscroll.viewport_size);
			else this->xscroll.viewport_offset = 0;
			if(this->yscroll.content_size > this->yscroll.viewport_size)
				this->yscroll.viewport_offset
					= min(this->yscroll.viewport_offset, this->yscroll.content_size - this->yscroll.viewport_size);
			else this->yscroll.viewport_offset = 0;
			for(uint64 i = 0; i < this->frames.rows * this->frames.columns; i++)
			{
				if(this->frames.addr[i].fm == nullptr) continue;
				this->frames.addr[i].fm->x -= int32(this->xscroll.viewport_offset);
				this->frames.addr[i].fm->y += int32(this->yscroll.viewport_offset);
				if(this->frames.addr[i].fm->x < + content_viewport.position.x + content_viewport.extent.x
					&& this->frames.addr[i].fm->x + int32(this->frames.addr[i].fm->width) >= + content_viewport.position.x
					&& this->frames.addr[i].fm->y < content_viewport.position.y + content_viewport.extent.y
					&& this->frames.addr[i].fm->y + int32(this->frames.addr[i].fm->height) >= content_viewport.position.y)
					this->frames.addr[i].fm->render(gd, bmp);
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

void grid_layout::insert_row(uint32 insert_idx, ui_size<float32> size)
{
	rows_desc.insert(insert_idx, size);
	frames.insert_rows(insert_idx, 1);
	for(uint64 j = 0; j < frames.columns; j++)
		frames.at(insert_idx, j).fm = nullptr;
}

void grid_layout::remove_row(uint32 remove_idx)
{
	rows_desc.remove(remove_idx);
	frames.remove_rows(remove_idx, 1);
}

void grid_layout::insert_column(uint32 insert_idx, ui_size<float32> size)
{
	columns_desc.insert(insert_idx, size);
	frames.insert_columns(insert_idx, 1);
	for(uint64 i = 0; i < frames.rows; i++)
		frames.at(i, insert_idx).fm = nullptr;
}

void grid_layout::remove_column(uint32 remove_idx)
{
	columns_desc.remove(remove_idx);
	frames.remove_columns(remove_idx, 1);
}

void grid_layout::update_layout()
{
	rectangle<int32> viewport = fm.viewport();
	rectangle<int32> content_viewport = fm.content_viewport();
	vector<uint32, 2> size;
	grid_layout_metrics metrics(this, content_viewport.extent.x, content_viewport.extent.y);
	grid_layout_evaluate_metrics(&metrics);
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
		for(uint64 i = 0; i < metrics.rows_size.size; i++)
			metrics.rows_size[i] = 0;
		for(uint64 i = 0; i < metrics.columns_size.size; i++)
			metrics.columns_size[i] = 0;
		grid_layout_evaluate_metrics(&metrics);
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
	grid_layout_frame *glf;
	vector<int32, 2> position = content_viewport.position;
	for(uint64 i = rows_desc.size - 1; i != uint64(-1); i--)
	{
		for(uint64 j = 0; j < columns_desc.size; j++)
		{
			glf = &frames.at(i, j);
			if(glf->fm != nullptr)
			{
				size = glf->fm->calculate_frame_size(metrics.columns_size[j], metrics.rows_size[i]);
				glf->fm->width = size.x;
				glf->fm->height = size.y;
				if(glf->halign == horizontal_align::left)
					glf->fm->x = position.x;
				else if(glf->halign == horizontal_align::center)
					glf->fm->x = position.x + (int32(metrics.columns_size[j]) - int32(glf->fm->width)) / 2;
				else glf->fm->x = position.x + int32(metrics.columns_size[j]) - int32(glf->fm->width);
				if(glf->valign == vertical_align::bottom)
					glf->fm->y = position.y;
				else if(glf->valign == vertical_align::center)
					glf->fm->y = position.y + (int32(metrics.rows_size[i]) - int32(glf->fm->height)) / 2;
				else glf->fm->y = position.y + int32(metrics.rows_size[i]) - int32(glf->fm->height);
			}
			position.x += int32(metrics.columns_size[j]);
		}
		position.x = content_viewport.position.x;
		position.y += int32(metrics.rows_size[i]);
	}
}
