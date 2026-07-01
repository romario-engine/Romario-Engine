#include "ui/window.h"
#include "global/os_api.h"
#include "ui/ui.h"

frame *window_mouse_event_receiver(window *wnd, frame *fm, observer<> *obs)
{
	if(obs == &wnd->fm.mouse_click)
	{
		wnd = wnd;
	}
	if(!fm->content_hit_test(mouse()->position)
		|| obs == &wnd->fm.mouse_click && fm->hook_mouse_click
		|| obs == &wnd->fm.mouse_release && fm->hook_mouse_release
		|| obs == &wnd->fm.mouse_move && fm->hook_mouse_move
		|| obs == &wnd->fm.mouse_wheel_rotate && fm->hook_mouse_wheel_rotate)
		return fm;
	frame *target = nullptr;
	array<frame *> frames;
	fm->subframes(&frames);
	for(uint64 i = 0; i < frames.size; i++)
	{
		if(frames[i]->visible
			&& frames[i]->hit_test(mouse()->position))
			target = window_mouse_event_receiver(wnd, frames[i], obs);
	}
	if(target == nullptr
		|| obs == &wnd->fm.mouse_click && target->return_mouse_click
		|| obs == &wnd->fm.mouse_release && target->return_mouse_release
		|| obs == &wnd->fm.mouse_move && target->return_mouse_move
		|| obs == &wnd->fm.mouse_wheel_rotate && target->return_mouse_wheel_rotate)
		return fm;
	else return target;
}

window::window()
{
	fm.width_desc = 600.0uiabs;
	fm.height_desc = 400.0uiabs;
	fm.width = 0;
	fm.height = 0;
	fm.mouse_click.callbacks.move_back(function<void()>([this] () -> void
		{
			frame *receiver = window_mouse_event_receiver(this, this->layout, &this->fm.mouse_click);
			if(ui()->option_list_tree.size != 0
				&& receiver != ui()->option_list_tree[0]
				&& !ui()->option_list_tree[0]->is_predecessor_of(receiver))
				ui()->option_list_tree.clear();
			if(receiver->focusable) ui()->focus_frame(receiver);
			else ui()->focus_frame(nullptr);
			receiver->mouse_click.trigger();
		}));
	fm.mouse_release.callbacks.move_back(function<void()>([this] () -> void
		{
			frame *receiver = window_mouse_event_receiver(this, this->layout, &this->fm.mouse_release);
			receiver->mouse_release.trigger();
		}));
	fm.mouse_move.callbacks.move_back(function<void()>([this] () -> void
		{
			frame *receiver = window_mouse_event_receiver(this, this->layout, &this->fm.mouse_move);
			ui()->hover_frame(receiver);
			receiver->mouse_move.trigger();
		}));
	fm.mouse_wheel_rotate.callbacks.move_back(function<void()>([this] () -> void
		{
			frame *receiver = window_mouse_event_receiver(this, this->layout, &this->fm.mouse_wheel_rotate);
			receiver->mouse_wheel_rotate.trigger();
		}));
	fm.key_press.callbacks.move_back(function<void()>([this] () -> void
		{
			if(ui()->focused_frame != nullptr)
				ui()->focused_frame->key_press.trigger();
		}));
	fm.key_release.callbacks.move_back(function<void()>([this] () -> void
		{
			if(ui()->focused_frame != nullptr)
				ui()->focused_frame->key_release.trigger();
		}));
	fm.char_input.callbacks.move_back(function<void()>([this] () -> void
		{
			if(ui()->focused_frame != nullptr)
				ui()->focused_frame->char_input.trigger();
		}));
	os_create_window(this);
}

window::~window()
{
	os_destroy_window(this);
}

void window::update()
{
	vector<int32, 2> position = os_window_content_position(this);
	vector<uint32, 2> client_size = os_window_content_size(this),
		screen_size = os_screen_size();
	if(bmp.width != screen_size.x || bmp.height != screen_size.y)
	{
		bmp.resize(screen_size.x, screen_size.y);
		os_update_window_size(this);
		for(uint32 i = 0; i < bmp.width * bmp.height; i++)
			bmp.data[i] = color(0, 0, 0, 0);
	}
	fm.x = position.x;
	fm.y = position.y;
	fm.width = client_size.x;
	fm.height = client_size.y;
	layout->x = position.x;
	layout->y = position.y;
	layout->width = fm.width;
	layout->height = fm.height;
	set_memory(bmp.data, bmp.width * bmp.height * sizeof(color), 0);
	graphics_displayer bp;
	layout->render(&bp, &bmp);
	//os_render_window(this);
}

void window::open()
{
	os_open_window(this);
}

void window::close()
{
	
}

void window::hide()
{

}

void window::resize(uint32 width, uint32 height)
{
	os_resize_window(this, width, height);
}
