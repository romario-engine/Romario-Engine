#pragma once
#include "ui/frame.h"

/*UI element. Must be allocated in dynamic memory.*/
struct window
{
	frame fm;
	bitmap bmp;
	frame *layout;
	void *handler;

	window();
	~window();
	void update();
	void open();
	void close();
	void hide();
	/*<layout> must be set*/
	void resize(uint32 width, uint32 height);
};
