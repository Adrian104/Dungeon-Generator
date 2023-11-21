// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace ini
{
	using section_type = std::map<std::string, std::string>;
	using container_type = std::map<std::string, section_type>;

	std::string Trim(const std::string& str);

	void Load(container_type& container, const std::string& path);
	void Save(const container_type& container, const std::string& path);
	void UserInput(section_type& section, const std::string& key);

	template <typename Type>
	Type Convert(const std::string& str)
	{
		if constexpr (std::is_same_v<Type, bool>)
		{
			switch (str.at(0))
			{
			case '0': case 'f': case 'F': case 'n': case 'N':
				return false;

			case '1': case 't': case 'T': case 'y': case 'Y':
				return true;

			default:
				throw std::invalid_argument("invalid_argument");
			}
		}
		else if constexpr (std::is_same_v<Type, std::string>) { return str; }
		else if constexpr (std::is_floating_point_v<Type>) { return static_cast<Type>(std::stof(str)); }
		else if constexpr (std::is_integral_v<Type>) { return static_cast<Type>(std::stoi(str)); }
		else { static_assert(!sizeof(Type*)); }
	}

	template <typename Type>
	void Get(section_type& section, const std::string& key, Type& out, bool req, bool (*check)(const Type&) = nullptr)
	{
		while (true)
		{
			const auto iter = section.find(key);
			if (iter != section.end() && iter->second.size() > 0)
			{
				try
				{
					Type temp = Convert<Type>(iter->second);
					if (check == nullptr || check(temp))
					{
						out = std::move(temp);
						break;
					}
				}
				catch (...) {}
			}

			if (!req)
				break;

			UserInput(section, key);
		}
	}

	template <typename Type>
	void Get(container_type& container, const std::string& sectionName, const std::string& key, Type& out, bool req, bool (*check)(const Type&) = nullptr)
	{
		const auto iter = container.find(sectionName);

		if (iter != container.end())
			Get<Type>(iter->second, key, out, req, check);
		else if (req)
			Get<Type>(container[sectionName], key, out, req, check);
	}
}