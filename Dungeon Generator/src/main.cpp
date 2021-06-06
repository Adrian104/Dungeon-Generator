#include "pch.hpp"
#include "app.hpp"

int main(int argc, char **argv)
{
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	Application app;
	app.Run();

	return EXIT_SUCCESS;
}