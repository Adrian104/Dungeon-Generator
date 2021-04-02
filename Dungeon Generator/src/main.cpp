#include "manager.hpp"

int main(int argc, char **argv)
{
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#else
	srand(unsigned(time(0)));
	#endif

	DGManager mgr;
	mgr.Run();

	return EXIT_SUCCESS;
}