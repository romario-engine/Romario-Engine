#include "data/array.h"
#include "tests.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "global/os_api.h"
#include <Windows.h>

int __stdcall wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow)
{
	//test_array();
	//test_string();
	//test_vector();
	//test_matrix();
	//test_real();
	//test_linear_algebra();
	test_ui();
	//test_set();
	//test_file();
	//test_fileset();
	//test_json();

	os_message_loop();

	return 0;
}
