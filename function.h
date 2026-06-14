#pragma once

template<typename return_type, typename... args_type>
return_type invoke_simple(void *storage, args_type... args)
{
	return ((return_type(*)(args_type...))(storage))(args...);
}

template<typename function_type, typename return_type, typename... args_type>
return_type invoke_lambda(void *storage, args_type... args)
{
	return (*((function_type *)(storage)))(args...);
}

template<typename type> struct function {};

template<typename return_type, typename... args_type>
struct function<return_type(args_type...)>
{
	void *storage;
	uint64 lambda_size;
	return_type(*invoke)(void *storage, args_type... args);

	function()
	{
		lambda_size = 0;
	}

	function(return_type(*f)(args_type...))
	{
		lambda_size = 0;
		assign_addr(f);
	}

	function(const function &f)
	{
		if(f.lambda_size == 0)
		{
			storage = f.storage;
			lambda_size = 0;
			invoke = f.invoke;
		}
		else
		{
			storage = new byte[f.lambda_size];
			copy_memory(f.storage, storage, f.lambda_size);
			lambda_size = f.lambda_size;
			invoke = f.invoke;
		}
	}

	function(function &&f)
	{
		storage = f.storage;
		lambda_size = f.lambda_size;
		invoke = f.invoke;
		f.lambda_size = 0;
	}

	template<typename function_type> function(const function_type &f)
	{
		lambda_size = 0;
		assign_lambda(f);
	}

	~function()
	{
		if(lambda_size != 0) delete[] storage;
	}

	void assign_addr(return_type(*f)(args_type...))
	{
		if(lambda_size != 0)
		{
			delete[] storage;
			lambda_size = 0;
		}
		storage = f;
		invoke = invoke_simple<return_type, args_type...>;
	}

	template<typename function_type> void assign_lambda(const function_type &f)
	{
		if(lambda_size != 0) delete[] storage;
		lambda_size = sizeof(function_type);
		storage = new byte[sizeof(function_type)];
		copy_memory(&f, storage, sizeof(function_type));
		invoke = invoke_lambda<function_type, return_type, args_type...>;
	}

	bool operator==(const function &f) const
	{
		if(lambda_size != f.lambda_size) return false;
		if(lambda_size == 0) return storage == f.storage;
		else return compare_memory(storage, f.storage, lambda_size) == compare_result::equal;
	}

	bool operator!=(const function &f) const
	{
		return !(*this == f);
	}

	return_type operator()(args_type... args) const
	{
		return invoke(storage, args...);
	}
};
