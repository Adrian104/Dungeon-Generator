#include <iostream>
#include <string>
#include "dgen.hpp"

int main(int argc, char **argv)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	DGManager mgr;
	mgr.Run();

	return EXIT_SUCCESS;
}