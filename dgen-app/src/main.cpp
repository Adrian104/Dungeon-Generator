#include "app.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int, char**)
{
	try
	{
		Application app;
		app.Run();
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}