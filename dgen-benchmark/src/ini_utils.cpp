// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "ini_utils.hpp"
#include <fstream>
#include <iostream>

namespace ini
{
	std::string Trim(const std::string& str)
	{
		static constexpr auto s_whitespaces = " \t\n\r\f\v";

		const auto first = str.find_first_not_of(s_whitespaces);
		if (first == std::string::npos) return "";

		const auto last = str.find_last_not_of(s_whitespaces);
		return str.substr(first, last - first + 1);
	}

	void Load(container_type& container, const std::string& path)
	{
		std::ifstream file(path);
		if (!file.good())
		{
			std::cerr << " Could not open '" << path << "' file\n";
			return;
		}

		std::string line;
		section_type* section = &(container[""]);

		while (!file.eof())
		{
			std::getline(file, line);
			size_t a, b;

			a = line.find_first_of(";#");
			if (a != std::string::npos)
				line.erase(a);

			a = line.find_first_of('[');
			if (a != std::string::npos)
			{
				b = line.find_last_of(']');
				if (b != std::string::npos && a < b)
				{
					section = &(container[Trim(line.substr(a + 1, b - a - 1))]);
					continue;
				}
			}

			a = line.find_first_of('=');
			if (a != std::string::npos)
			{
				const std::string key = Trim(line.substr(0, a));
				if (!key.empty())
					(*section)[key] = Trim(line.substr(a + 1));
			}
		}
	}

	void Save(const container_type& container, const std::string& path)
	{
		std::ofstream file(path);
		if (!file.good()) return;

		bool next = false;
		for (const auto& [sectionName, section] : container)
		{
			if (next) file << '\n';
			next = true;

			file << '[' << sectionName << "]\n";

			for (const auto& [key, value] : section)
				file << key << " = " << value << '\n';
		}
	}

	void UserInput(section_type& section, const std::string& key)
	{
		std::cout << ' ' << key << " = ";

		std::string input;
		std::getline(std::cin, input);

		section[key] = Trim(input);
	}
}