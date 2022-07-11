#pragma once
#include "../app.hpp"
#include <functional>
#include <string>

class Warning : public Widget, public Animator
{
	int m_width = 0;
	int m_height = 0;

	SDL_Texture* m_renderOutput = nullptr;
	size_t m_hash = std::hash<std::string>()("");

	void Clear();

	public:
	Warning(Application& app) : Widget(app), Animator(g_warningAnimTime) {}
	~Warning() { Clear(); }

	void Draw() override;
	void Set(const std::string& msg);
};