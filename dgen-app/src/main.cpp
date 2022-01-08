#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include "app.hpp"

int main(int argc, char **argv)
{
	try
	{
		Application app;
		app.Run();
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}