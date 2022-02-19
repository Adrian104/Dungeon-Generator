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

	virtual std::string GetValue() const = 0;
};

struct FactorMod : public Modifier
{
	float& m_ref;
	float m_step;

	FactorMod(const std::string& name, float& ref, float step = 0.1f) : Modifier(name), m_ref(ref), m_step(step) {}

	void Increment() override { m_ref *= (1.0f + m_step); }
	void Decrement() override { m_ref *= (1.0f - m_step); }

	std::string GetValue() const override { return std::to_string(m_ref); }
};

struct PercentMod : public Modifier
{
	float& m_ref;
	float m_step;

	PercentMod(const std::string& name, float& ref, float step = 0.05f) : Modifier(name), m_ref(ref), m_step(step) {}

	void Increment() override { m_ref += m_step; if (m_ref > 1.0f) m_ref = 1.0f; }
	void Decrement() override { m_ref -= m_step; if (m_ref < 0.0f) m_ref = 0.0f; }

	std::string GetValue() const override { return std::to_string(static_cast<int>(std::round(m_ref * 100.0f))) + " %"; }
};

struct BoolMod : public Modifier
{
	bool& m_ref;

	BoolMod(const std::string& name, bool& ref) : Modifier(name), m_ref(ref) {}

	void Increment() override { m_ref = !m_ref; }
	void Decrement() override { m_ref = !m_ref; }

	std::string GetValue() const override { return m_ref ? "Enabled" : "Disabled"; }
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

	std::string GetValue() const override { return std::to_string(m_ref); }
};