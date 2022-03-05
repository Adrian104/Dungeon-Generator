#pragma once
#include <SDL2/SDL.h>

class Animator;
class Application;

class Widget
{
	protected:
	Application& m_app;

	private:
	bool m_render = true;
	Widget* m_next = nullptr;
	Animator* m_animator = nullptr;

	protected:
	Widget(Application& app) : m_app(app) {}

	public:
	virtual ~Widget() { delete m_next; }

	virtual void Draw() {}
	virtual void Render() {}
	virtual void HandleEvent(SDL_Event& sdlEvent) {}

	Widget(const Widget& ref) = delete;
	Widget& operator=(const Widget& ref) = delete;

	Widget(Widget&& ref) noexcept = delete;
	Widget& operator=(Widget&& ref) noexcept = delete;

	void ScheduleRendering() { m_render = true; }
	friend class Application;
};