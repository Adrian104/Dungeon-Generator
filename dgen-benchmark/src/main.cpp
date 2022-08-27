#include "benchmark.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	try
	{
		Benchmark benchmark;
		benchmark.Run();
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}