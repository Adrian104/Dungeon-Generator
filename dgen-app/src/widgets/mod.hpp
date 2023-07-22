// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <cmath>
#include <limits>
#include <string>

struct Modifier
{
	std::string m_name;

	Modifier(const std::string& name) : m_name(name) {}
	virtual ~Modifier() = default;

	virtual void Increment() = 0;
	virtual void Decrement() = 0;

	virtual void Check() = 0;
	virtual std::string GetValue() const = 0;
	virtual void SetValue(const std::string& val) = 0;
};

struct FactorMod : public Modifier
{
	float& m_ref;
	float m_step;

	FactorMod(const std::string& name, float& ref, float step = 0.1f) : Modifier(name), m_ref(ref), m_step(step) {}

	void Increment() override { m_ref *= (1.0f + m_step); }
	void Decrement() override { m_ref *= (1.0f - m_step); }

	void Check() override { if (m_ref <= 0) m_ref = std::numeric_limits<float>::min(); }
	std::string GetValue() const override { return std::to_string(m_ref); }
	void SetValue(const std::string& val) override { m_ref = std::stof(val); }
};

struct PercentMod : public Modifier
{
	float& m_ref;
	float m_step;

	PercentMod(const std::string& name, float& ref, float step = 0.05f) : Modifier(name), m_ref(ref), m_step(step) {}

	void Increment() override { m_ref += m_step; if (m_ref > 1.0f) m_ref = 1.0f; }
	void Decrement() override { m_ref -= m_step; if (m_ref < 0.0f) m_ref = 0.0f; }

	void Check() override { if (m_ref < 0) m_ref = 0; else if (m_ref > 1.0f) m_ref = 1.0f; }
	std::string GetValue() const override { return std::to_string(std::lround(m_ref * 100.0f)) + " %"; }
	void SetValue(const std::string& val) override { m_ref = std::stof(val) / 100.0f; }
};

struct BoolMod : public Modifier
{
	bool& m_ref;

	BoolMod(const std::string& name, bool& ref) : Modifier(name), m_ref(ref) {}

	void Increment() override { m_ref = !m_ref; }
	void Decrement() override { m_ref = !m_ref; }

	void Check() override {}
	std::string GetValue() const override { return m_ref ? "Enabled" : "Disabled"; }
	void SetValue(const std::string& val) override { m_ref = val[0] == 't' || val[0] == 'e' || val[0] == '1'; }
};

struct IntMod : public Modifier
{
	int& m_ref;
	int m_min = 0;
	int m_max = std::numeric_limits<int>::max();

	IntMod(const std::string& name, int& ref) : Modifier(name), m_ref(ref) {}
	IntMod(const std::string& name, int& ref, int min, int max) : Modifier(name), m_ref(ref), m_min(min), m_max(max) {}

	void Increment() override { m_ref += m_ref < m_max; }
	void Decrement() override { m_ref -= m_ref > m_min; }

	void Check() override { if (m_ref < m_min) m_ref = m_min; else if (m_ref > m_max) m_ref = m_max; }
	std::string GetValue() const override { return std::to_string(m_ref); }
	void SetValue(const std::string& val) override { m_ref = std::stoi(val); }
};