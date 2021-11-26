#include "pch.hpp"
#include "app.hpp"

int main(int argc, char **argv)
{
	try
	{
		Application app;
		app.Run();
	}
	catch (const std::exception &error)
	{
		std::cout << error.what();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}