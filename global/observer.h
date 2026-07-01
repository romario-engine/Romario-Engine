#pragma once
#include "data/array.h"
#include "global/function.h"

template<typename ...args> struct observer
{
	array<function<void(args...)>> callbacks;

	void trigger(args... arguments)
	{
		for(uint64 i = 0; i < callbacks.size; i++)
		{
			callbacks[i](arguments...);
		}
	}
};
