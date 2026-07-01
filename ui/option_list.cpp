#include "ui/option_list.h"
#include "ui/ui.h"

option_list::option_list()
{
	open_anchor = option_list_open_anchor::bottom;
	open_upon_action = option_list_open_upon_action::none;
	button.button_click.assign_lambda([this] () -> void
		{
			if(this->open_upon_action == option_list_open_upon_action::click)
			{
				for(uint64 i = 0; i < ui()->option_list_tree.size; i++)
					if(ui()->option_list_tree[i] == &this->list.fm)
					{
						this->close();
						return;
					}
				this->open_at_frame(&this->button.fm);
			}
		});
	button.fm.start_hover.callbacks.move_back(function<void()>([this] () -> void
		{
			if(this->open_upon_action == option_list_open_upon_action::hover)
				this->open_at_frame(&this->button.fm);
		}));
}

void option_list::set_opening(option_list_open_upon_action opening)
{
	if(open_upon_action == option_list_open_upon_action::click)
		button.button_click.assign_addr(emit_empty_function<void()>());
	else if(open_upon_action == option_list_open_upon_action::hover)
		button.fm.start_hover.callbacks.remove_element(button_hover_callback);
	open_upon_action = opening;
	if(open_upon_action == option_list_open_upon_action::click)
		copy(button_click_callback, &button.button_click);
	else if(open_upon_action == option_list_open_upon_action::hover)
		button.fm.start_hover.callbacks.move_back(button_hover_callback);
}

void option_list::open_at_point(vector<int32, 2> point)
{
	vector<uint32, 2> size = list.fm.calculate_frame_size(ui()->width, ui()->height);
	list.fm.width = size.x;
	list.fm.height = size.y;
	vector<int32, 2> p = point;
	if(p.x + int32(list.fm.width) > ui()->width)
		p.x = ui()->width - int32(list.fm.width);
	if(p.x < 0) p.x = 0;
	if(p.y < 0) p.y = 0;
	if(p.y + int32(list.fm.height) > ui()->height)
		p.y = ui()->height - int32(list.fm.height);
	list.fm.x = p.x;
	list.fm.y = p.y;
	int64 start_remove;
	for(start_remove = ui()->option_list_tree.size - 1; start_remove >= 0; start_remove--)
		if(ui()->option_list_tree[start_remove]->is_predecessor_of(&button.fm)) break;
	start_remove++;
	if(start_remove < ui()->option_list_tree.size)
		ui()->option_list_tree.remove_range(start_remove, ui()->option_list_tree.size);
	ui()->option_list_tree.push_back(&list.fm);
}

void option_list::open_at_frame(frame *fm)
{
	vector<uint32, 2> size = list.fm.calculate_frame_size(ui()->width, ui()->height);
	list.fm.width = size.x;
	list.fm.height = size.y;
	if(open_anchor == option_list_open_anchor::left)
	{
		list.fm.x = fm->x - int32(list.fm.width);
		list.fm.y = fm->y + int32(fm->height) - int32(list.fm.height);
	}
	else if(open_anchor == option_list_open_anchor::right)
	{
		list.fm.x = fm->x + int32(fm->width);
		list.fm.y = fm->y + int32(fm->height) - int32(list.fm.height);
	}
	else if(open_anchor == option_list_open_anchor::bottom)
	{
		list.fm.x = fm->x;
		list.fm.y = fm->y - int32(list.fm.height);
	}
	else
	{
		list.fm.x = fm->x;
		list.fm.y = fm->y + int32(fm->height);
	}
	open_at_point(vector<int32, 2>(list.fm.x, list.fm.y));
}

void option_list::close()
{
	for(uint64 i = 0; i < ui()->option_list_tree.size; i++)
		if(ui()->option_list_tree[i] == &list.fm)
		{
			ui()->option_list_tree.remove_range(i, ui()->option_list_tree.size);
			break;
		}
}
