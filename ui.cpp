#include "ui.h"
#include "os_api.h"

user_interface::user_interface()
{
	vector<uint32, 2> size = os_screen_size();
	width = int32(size.x);
	height = int32(size.y);
	caret_timer.period = 500000000;
	caret_timer.callback.assign_lambda([this] () -> timer_trigger_postaction
		{
			caret_visible = !caret_visible;
			os_update_windows();
			return timer_trigger_postaction::reactivate;
		});
	caret_frame.width_desc = 0.0uiabs;
	caret_frame.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			if(!ui()->caret_visible) return;
			rectangle<int32> rect(
				vector<int32, 2>(this->caret_frame.x, this->caret_frame.y),
				vector<int32, 2>(2, this->caret_frame.height));
			gd->brush.switch_solid_color(color(0, 0, 0, 255));
			gd->fill_rect(rect, bmp);
		});
}

void user_interface::hover_frame(frame *fm)
{
	if(hovered_frame == fm) return;
	if(hovered_frame != nullptr)
		hovered_frame->end_hover.trigger();
	hovered_frame = fm;
	if(hovered_frame != nullptr)
		hovered_frame->start_hover.trigger();
}

void user_interface::focus_frame(frame *fm)
{
	if(focused_frame == fm) return;
	if(focused_frame != nullptr)
		focused_frame->focus_loss.trigger();
	focused_frame = fm;
	if(focused_frame != nullptr)
		focused_frame->focus_receive.trigger();
}

user_interface ui_instance;

user_interface *ui()
{
	return &ui_instance;
}
