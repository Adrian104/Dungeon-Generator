#pragma once
#include <vector>
#include "mod.hpp"
#include "widget.hpp"
#include "../anim.hpp"

class Menu : public Widget, public Animator
{
	int m_selection = 0;
	SDL_Texture* m_renderOutput;
	std::vector<Modifier*> m_mods;

	public:
	Menu(Application& app);
	~Menu();

	void Draw() override;
	void Render() override;
	void HandleEvent(SDL_Event& sdlEvent) override;

	template <typename Type>
	void Add(const Type& mod) { m_mods.push_back(new Type(mod)); }
};