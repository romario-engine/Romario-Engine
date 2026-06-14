#pragma once
#include "string.h"
#include "geometry.h"
#include "graphics.h"
#include "ui_types.h"
#include "frame.h"

enum struct glyph_type
{
	character,
	frame
};

struct character_data
{
	char32 code;
	geometry_path path;
	vector<int32, 2> advance;
	bitmap bmp;
	vector<float32, 2> bmp_offset;
	u16string font_name;
	uint32 size;
	bool italic;
	uint32 weight;
	uint32 ascent;
	uint32 descent;
	uint32 internal_leading;
	int32 underline_offset;
	uint32 underline_size;
	int32 strikethrough_offset;
	uint32 strikethrough_size;
};

struct glyph
{
	glyph_type type;
	character_data *data;
	bool underlined;
	bool strikedthrough;
	color text_color;
	color background_color;
	frame *fm;
};

enum struct text_background_fill
{
	glyph,
	line
};

struct text_layout
{
	array<glyph> glyphs;
	uint32 width;
	uint32 height;
	horizontal_align halign;
	vertical_align valign;
	bool multiline;
	text_background_fill tbf;

	text_layout();
	void insert_text(
		uint64 idx,
		const u16string &str,
		const u16string &font,
		uint32 font_size,
		bool italic = false,
		uint32 weight = 400,
		bool underlined = false,
		bool strikedthrough = false,
		color text_color = color(0, 0, 0, 255),
		color background_color = color(0, 0, 0, 0));
	void insert_frame(uint64 idx, frame *fm);
	vector<int32, 2> content_size();
	void hit_test_position(uint64 idx, vector<int32, 2> *point, int32 *line_height);
	void hit_test_point(vector<int32, 2> point, uint64 *idx);
	void render(graphics_displayer *gd, vector<int32, 2> point, bitmap *bmp);
};
