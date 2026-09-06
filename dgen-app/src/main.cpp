// SPDX-FileCopyrightText: Copyright (c) 2026 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "pch.hpp"

int main(int, char**)
{
	try
	{

	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "unknown error\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
