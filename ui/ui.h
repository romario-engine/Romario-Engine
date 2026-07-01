#pragma once
#include "data/array.h"
#include "ui/frame.h"
#include "global/timer.h"

struct user_interface
{
	int32 width;
	int32 height;
	frame *hovered_frame;
	frame *focused_frame;
	bool caret_visible;
	timer caret_timer;
	frame caret_frame;
	array<frame *> option_list_tree;

	user_interface();
	void hover_frame(frame *fm);
	void focus_frame(frame *fm);
};

user_interface *ui();
