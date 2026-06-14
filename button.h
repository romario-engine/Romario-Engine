#pragma once
#include "frame.h"
#include "text_field.h"
#include "flow_layout.h"
#include "function.h"

/*Must be allocated in dynamic memory.*/
struct radio_button_group
{
	frame *selected_frame;

	radio_button_group();
};

/*UI element. Must be allocated in dynamic memory.*/
struct button
{
	frame fm;
	flow_layout layout;
	function<void()> button_click;

	button();
	/*Text field must be deleted manually.*/
	void setup_text_button(text_field **tf);
	void setup_highlighted_button(
		color hovered_color = color(50, 150, 230, 255),
		color pressed_color = color(75, 175, 255, 255));
	/*Text field must be deleted manually.*/
	void setup_marked_radio_button(radio_button_group *group, frame **mark, text_field **tf);
	void setup_highlighted_radio_button(
		radio_button_group *group,
		color hovered_color = color(50, 150, 230, 255),
		color selected_color = color(75, 175, 255, 255));
	bool pressed();
};
