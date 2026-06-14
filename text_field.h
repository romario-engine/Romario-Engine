#pragma once
#include "frame.h"
#include "scroll_bar.h"
#include "string.h"
#include "text_layout.h"

/*UI element. Must be allocated in dynamic memory.*/
struct text_field
{
	frame fm;
	text_layout tl;
	scroll_bar scroll;
	uint64 position;
	uint64 select_position;
	bool selecting;
	bool editable;
	color background_selecting_color;
	u16string font;
	uint32 font_size;
	bool italic;
	uint32 weight;
	bool underlined;
	bool strikedthrough;
	color text_color;

	text_field(bool is_editable, bool is_multiline);
	void setup(bool is_editable, bool is_multiline);
	void update_position_by_mouse();
	void move(uint64 idx);
	void select(uint64 begin, uint64 end);
	void insert_text(const u16string &text);
	void insert_frame(frame *frame_addr);
	void remove();
	void scroll_to_caret();
};