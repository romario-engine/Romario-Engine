#include "text_layout.h"
#include "os_api.h"

template<> struct key<character_data *>
{
	char32 code;
	u16string font_name;
	uint32 size;
	bool italic;
	uint32 weight;

	key(character_data *const &value)
	{
		code = value->code;
		font_name << value->font_name;
		size = value->size;
		italic = value->italic;
		weight = value->weight;
	}

	key(char32 code, const u16string &font_name_value, uint32 size, bool italic, uint32 weight)
		: code(code), font_name(font_name_value), size(size), italic(italic), weight(weight) {}

	bool operator<(const key &value) const
	{
		if(code < value.code) return true;
		else if(code > value.code) return false;
		if(font_name < value.font_name) return true;
		else if(font_name > value.font_name) return false;
		if(size < value.size) return true;
		else if(size > value.size) return false;
		if(italic < value.italic) return true;
		else if(italic > value.italic) return false;
		if(weight < value.weight) return true;
		else if(weight > value.weight) return false;
		return false;
	}
};

array<character_data *> glyph_cache;
character_data *unknown_glyph = nullptr;

text_layout::text_layout()
{
	width = 0;
	height = 0;
	valign = vertical_align::top;
	halign = horizontal_align::left;
	multiline = true;
	tbf = text_background_fill::line;
}

void text_layout::insert_text(
	uint64 idx,
	const u16string &str,
	const u16string &font,
	uint32 font_size,
	bool italic,
	uint32 weight,
	bool underlined,
	bool strikedthrough,
	color text_color,
	color background_color)
{
	character_data *data;
	glyphs.insert_default(idx, str.size);
	for(uint64 i = 0, r; i < str.size; i++, idx++)
	{
		r = glyph_cache.binary_search(key<character_data *>(
			char32(str[i]),
			font,
			font_size,
			italic,
			weight));
		if(r == glyph_cache.size)
		{
			data = new character_data();
			data->code = char32(str[i]);
			data->font_name << font;
			data->size = font_size,
			data->italic = italic;
			data->weight = weight;
			if(os_load_glyph(data))
			{
				glyph_cache.binary_insert(data);
				glyphs[idx].data = data;
			}
			else
			{
				delete data;
				if(unknown_glyph == nullptr)
				{
					unknown_glyph = new character_data();
					unknown_glyph->size = 0;
					unknown_glyph->bmp.resize(0, 0);
					unknown_glyph->advance = vector<int32, 2>(0, 0);
					unknown_glyph->ascent = 0;
					unknown_glyph->descent = 0;
					unknown_glyph->internal_leading = 0;
					unknown_glyph->strikethrough_offset = 0;
					unknown_glyph->strikethrough_size = 0;
					unknown_glyph->underline_offset = 0;
					unknown_glyph->underline_size = 0;
				}
				glyphs[idx].data = unknown_glyph;
			}
		}
		else glyphs[idx].data = glyph_cache[r];
		glyphs[idx].type = glyph_type::character;
		glyphs[idx].underlined = underlined;
		glyphs[idx].strikedthrough = strikedthrough;
		glyphs[idx].text_color = text_color;
		glyphs[idx].background_color = background_color;
	}
}

void text_layout::insert_frame(uint64 idx, frame *fm)
{
	glyphs.insert_default(idx, 1);
	glyphs[idx].type = glyph_type::frame;
	glyphs[idx].fm = fm;
}

vector<int32, 2> text_layout::content_size()
{
	vector<int32, 2> size(0, 0);
	int32 line_width = 0, line_height = 0;
	uint64 line_begin = 0, line_end = 0;
	for(uint64 i = 0; i <= glyphs.size; i++)
	{
		if(glyphs[i].type == glyph_type::frame)
		{
			vector<uint32, 2> size = glyphs[i].fm->calculate_frame_size(width, height);
			glyphs[i].fm->width = size.x;
			glyphs[i].fm->height = size.y;
		}
		if(i == glyphs.size
			|| multiline
			&& (glyphs[i].type == glyph_type::character
				&& (glyphs[i].data->code == U'\n' || line_width + glyphs[i].data->advance.x > int32(width))
				|| glyphs[i].type == glyph_type::frame
				&& line_width + glyphs[i].fm->width > int32(width)))
		{
			if(glyphs[i].type == glyph_type::character && i == glyphs.size)
				line_end = i;
			else if(glyphs[i].type == glyph_type::character && glyphs[i].data->code == U'\n')
				line_end = i + 1;
			else
			{
				if(line_begin != i) line_end = i;
				else line_end = i + 1;
			}
			line_width = 0;
			line_height = 0;
			for(uint64 j = line_begin; j < line_end; j++)
			{
				if(glyphs[j].type == glyph_type::character)
				{
					line_height = max(
						line_height,
						int32(glyphs[j].data->internal_leading
							+ glyphs[j].data->ascent
							+ glyphs[j].data->descent));
					line_width += glyphs[j].data->advance.x;
				}
				else
				{
					line_height = max(line_height, glyphs[j].fm->height);
					line_width += glyphs[j].fm->width;
				}
			}
			size.x = max(size.x, line_width);
			size.y += line_height;
			line_begin = line_end;
			if(line_begin == glyphs.size) break;
			i = line_end - 1;
			line_width = 0;
		}
		else
		{
			if(glyphs[i].type == glyph_type::character)
				line_width += glyphs[i].data->advance.x;
			else line_width += glyphs[i].fm->width;
		}
	}
	return size;
}

void text_layout::hit_test_position(uint64 idx, vector<int32, 2> *point, int32 *line_height)
{
	vector<int32, 2> p(0, int32(height)),
		text_size = content_size();
	if(valign == vertical_align::center && int32(height) >= text_size.y)
		p.y = (int32(height) + text_size.y) / 2;
	else if(valign == vertical_align::bottom && int32(height) >= text_size.y)
		p.y = text_size.y;
	int32 line_width = 0;
	uint64 line_begin = 0, line_end = 0;
	*line_height = 0;
	for(uint64 i = 0; i <= glyphs.size; i++)
	{
		if(glyphs[i].type == glyph_type::frame)
		{
			vector<uint32, 2> size = glyphs[i].fm->calculate_frame_size(width, height);
			glyphs[i].fm->width = size.x;
			glyphs[i].fm->height = size.y;
		}
		if(i == glyphs.size
			|| multiline
			&& (glyphs[i].type == glyph_type::character
				&& (glyphs[i].data->code == U'\n' || line_width + glyphs[i].data->advance.x > int32(width))
				|| glyphs[i].type == glyph_type::frame
				&& line_width + glyphs[i].fm->width > int32(width)))
		{
			if(glyphs[i].type == glyph_type::character && i == glyphs.size)
				line_end = i;
			else if(glyphs[i].type == glyph_type::character && glyphs[i].data->code == U'\n')
				line_end = i + 1;
			else
			{
				if(line_begin != i) line_end = i;
				else line_end = i + 1;
			}
			line_width = 0;
			*line_height = 0;
			for(uint64 j = line_begin; j < line_end; j++)
			{
				if(glyphs[j].type == glyph_type::character)
				{
					*line_height = max(
						*line_height,
						int32(glyphs[j].data->internal_leading
							+ glyphs[j].data->ascent
							+ glyphs[j].data->descent));
					line_width += glyphs[j].data->advance.x;
				}
				else
				{
					*line_height = max(*line_height, glyphs[j].fm->height);
					line_width += glyphs[j].fm->width;
				}
			}
			p.y -= *line_height;
			if(halign == horizontal_align::center && int32(width) >= line_width)
				p.x = (int32(width) - line_width) / 2;
			else if(halign == horizontal_align::right && int32(width) >= line_width)
				p.x = int32(width) - line_width;
			point->y = p.y;
			for(uint64 j = line_begin; j < line_end; j++)
			{
				if(j == idx)
				{
					point->x = p.x;
					return;
				}
				if(glyphs[j].type == glyph_type::character)
					p.x += glyphs[j].data->advance.x;
				else p.x += glyphs[j].fm->width;
				if(j + 1 == idx && idx == glyphs.size)
				{
					point->x = p.x;
					return;
				}
			}
			p.x = 0;
			line_begin = line_end;
			if(line_begin == glyphs.size) return;
			i = line_end - 1;
			line_width = 0;
		}
		else
		{
			if(glyphs[i].type == glyph_type::character)
				line_width += glyphs[i].data->advance.x;
			else line_width += glyphs[i].fm->width;
		}
	}
}

void text_layout::hit_test_point(vector<int32, 2> point, uint64 *idx)
{
	if(glyphs.size == 0)
	{
		*idx = 0;
		return;
	}
	vector<int32, 2> p(0, int32(height)), text_size = content_size();
	if(valign == vertical_align::center && int32(height) >= text_size.y)
		p.y = (int32(height) + text_size.y) / 2;
	else if(valign == vertical_align::bottom && int32(height) >= text_size.y)
		p.y = text_size.y;
	int32 line_width = 0, line_height = 0;
	uint64 line_begin = 0, line_end = 0;
	for(uint64 i = 0; i <= glyphs.size; i++)
	{
		if(glyphs[i].type == glyph_type::frame)
		{
			vector<uint32, 2> size = glyphs[i].fm->calculate_frame_size(width, height);
			glyphs[i].fm->width = size.x;
			glyphs[i].fm->height = size.y;
		}
		if(i == glyphs.size
			|| multiline
			&& (glyphs[i].type == glyph_type::character
				&& (glyphs[i].data->code == U'\n' || line_width + glyphs[i].data->advance.x > int32(width))
				|| glyphs[i].type == glyph_type::frame
				&& line_width + glyphs[i].fm->width > int32(width)))
		{
			if(glyphs[i].type == glyph_type::character && i == glyphs.size)
				line_end = i;
			else if(glyphs[i].type == glyph_type::character && glyphs[i].data->code == U'\n')
				line_end = i + 1;
			else
			{
				if(line_begin != i) line_end = i;
				else line_end = i + 1;
			}
			line_width = 0;
			line_height = 0;
			for(uint64 j = line_begin; j < line_end; j++)
			{
				if(glyphs[j].type == glyph_type::character)
				{
					line_height = max(
						line_height,
						int32(glyphs[j].data->internal_leading
							+ glyphs[j].data->ascent
							+ glyphs[j].data->descent));
					line_width += glyphs[j].data->advance.x;
				}
				else
				{
					line_height = max(line_height, glyphs[j].fm->height);
					line_width += glyphs[j].fm->width;
				}
			}
			p.y -= line_height;
			p.x = 0;
			if(halign == horizontal_align::center && int32(width) >= line_width)
				p.x = (int32(width) - line_width) / 2;
			else if(halign == horizontal_align::right && int32(width) >= line_width)
				p.x = int32(width) - line_width;
			if(point.y >= p.y)
			{
				for(uint64 j = line_begin; j < line_end; j++)
				{
					if(glyphs[j].type == glyph_type::character)
					{
						if(point.x <= p.x + glyphs[j].data->advance.x / 2)
						{
							*idx = j;
							return;
						}
						p.x += glyphs[j].data->advance.x;
					}
					else
					{
						if(point.x <= p.x + glyphs[j].fm->width / 2)
						{
							*idx = j;
							return;
						}
						p.x += glyphs[j].fm->width;
					}
				}
				*idx = line_end;
				return;
			}
			line_begin = line_end;
			if(line_begin == glyphs.size)
			{
				*idx = glyphs.size;
				return;
			}
			i = line_end - 1;
			line_width = 0;
		}
		else
		{
			if(glyphs[i].type == glyph_type::character)
				line_width += glyphs[i].data->advance.x;
			else line_width += glyphs[i].fm->width;
		}
	}
}

void text_layout::render(graphics_displayer *gd, vector<int32, 2> point, bitmap *bmp)
{
	if(glyphs.size == 0) return;
	vector<int32, 2> p(point.x, point.y + int32(height)),
		text_size = content_size();
	if(valign == vertical_align::center && int32(height) >= text_size.y)
		p.y = point.y + (int32(height) + text_size.y) / 2;
	else if(valign == vertical_align::bottom && int32(height) >= text_size.y)
		p.y = point.y + text_size.y;
	int32 baseline = 0, line_width = 0, line_height = 0;
	uint64 line_begin = 0, line_end = 0;
	for(uint64 i = 0; i <= glyphs.size; i++)
	{
		if(glyphs[i].type == glyph_type::frame)
		{
			vector<uint32, 2> size = glyphs[i].fm->calculate_frame_size(width, height);
			glyphs[i].fm->width = size.x;
			glyphs[i].fm->height = size.y;
		}
		if(i == glyphs.size
			|| multiline
			&& (glyphs[i].type == glyph_type::character
				&& (glyphs[i].data->code == U'\n' || line_width + glyphs[i].data->advance.x > int32(width))
				|| glyphs[i].type == glyph_type::frame
				&& line_width + glyphs[i].fm->width > int32(width)))
		{
			if(glyphs[i].type == glyph_type::character && i == glyphs.size)
				line_end = i;
			else if(glyphs[i].type == glyph_type::character && glyphs[i].data->code == U'\n')
				line_end = i + 1;
			else
			{
				if(line_begin != i) line_end = i;
				else line_end = i + 1;
			}
			line_width = 0;
			line_height = 0;
			baseline = 0;
			for(uint64 j = line_begin; j < line_end; j++)
			{
				if(glyphs[j].type == glyph_type::character)
				{
					line_height = max(
						line_height,
						int32(glyphs[j].data->internal_leading
							+ glyphs[j].data->ascent
							+ glyphs[j].data->descent));
					line_width += glyphs[j].data->advance.x;
					baseline = max(baseline, int32(glyphs[j].data->descent));
				}
				else
				{
					line_height = max(line_height, glyphs[j].fm->height);
					line_width += glyphs[j].fm->width;
				}
			}
			p.y -= line_height - baseline;
			p.x = point.x;
			if(halign == horizontal_align::center && int32(width) >= line_width)
				p.x = point.x + (int32(width) - line_width) / 2;
			else if(halign == horizontal_align::right && int32(width) >= line_width)
				p.x = point.x + int32(width) - line_width;
			for(uint64 j = line_begin; j < line_end; j++)
			{
				if(glyphs[j].type == glyph_type::character)
				{
					if(gd->scissor_stack.size != 0
						&& (p.x + int32(glyphs[j].data->bmp_offset.x)
								>= gd->scissor_stack.back().position.x + gd->scissor_stack.back().extent.x
							|| p.x + int32(glyphs[j].data->bmp_offset.x) + glyphs[j].data->bmp.width <= gd->scissor_stack.back().position.x
							|| p.y + int32(glyphs[j].data->bmp_offset.y)
								>= gd->scissor_stack.back().position.y + gd->scissor_stack.back().extent.y
							|| p.y + int32(glyphs[j].data->bmp_offset.y) + glyphs[j].data->bmp.height <= gd->scissor_stack.back().position.y))
					{
						p.x += glyphs[j].data->advance.x;
						continue;
					}
					rectangle<int32> rect;
					if(glyphs[j].background_color.a != 0)
					{
						if(tbf == text_background_fill::glyph)
						{
							rect.position = vector<int32, 2>(p.x + glyphs[j].data->bmp_offset.x, p.y + glyphs[j].data->bmp_offset.y);
							rect.extent = vector<int32, 2>(glyphs[j].data->bmp.width, glyphs[j].data->bmp.height);
						}
						else
						{
							rect.position = vector<int32, 2>(p.x, p.y - baseline);
							rect.extent = vector<int32, 2>(glyphs[j].data->advance.x, line_height);
						}
						gd->brush.switch_solid_color(glyphs[j].background_color);
						gd->fill_rect(rect, bmp);
					}
					gd->brush.switch_solid_color(glyphs[j].text_color);
					gd->fill_opacity_bitmap(
						glyphs[j].data->bmp,
						vector<int32, 2>(
							p.x + int32(glyphs[j].data->bmp_offset.x),
							p.y + int32(glyphs[j].data->bmp_offset.y)),
						bmp);
					if(glyphs[j].underlined)
					{
						rect.position = vector<int32, 2>(p.x, p.y + glyphs[j].data->underline_offset);
						rect.extent = vector<int32, 2>(glyphs[j].data->advance.x, glyphs[j].data->underline_size);
						gd->fill_rect(rect, bmp);
					}
					if(glyphs[j].strikedthrough)
					{
						rect.position = vector<int32, 2>(p.x, p.y + glyphs[j].data->strikethrough_offset);
						rect.extent = vector<int32, 2>(glyphs[j].data->advance.x, glyphs[j].data->strikethrough_size);
						gd->fill_rect(rect, bmp);
					}
					p.x += glyphs[j].data->advance.x;
				}
				else
				{
					glyphs[j].fm->x = p.x;
					glyphs[j].fm->y = p.y - baseline;
					if(gd->scissor_stack.size != 0
						&& (glyphs[j].fm->x > gd->scissor_stack.back().position.x + gd->scissor_stack.back().extent.x
							|| glyphs[j].fm->x + glyphs[j].fm->width < gd->scissor_stack.back().position.x
							|| glyphs[j].fm->y >= gd->scissor_stack.back().position.y + gd->scissor_stack.back().extent.y
							|| glyphs[j].fm->y + glyphs[j].fm->height <= gd->scissor_stack.back().position.y))
					{
						p.x += glyphs[j].fm->width;
						continue;
					}
					glyphs[j].fm->render(gd, bmp);
					p.x += glyphs[j].fm->width;
				}
			}
			p.y -= baseline;
			line_begin = line_end;
			if(line_begin == glyphs.size) return;
			i = line_end - 1;
			line_width = 0;
		}
		else
		{
			if(glyphs[i].type == glyph_type::character)
				line_width += glyphs[i].data->advance.x;
			else line_width += glyphs[i].fm->width;
		}
	}
}
