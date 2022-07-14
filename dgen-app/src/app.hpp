#pragma once
#include "anim.hpp"
#include "appmgr.hpp"
#include "dgen.hpp"
#include "global.hpp"
#include "vport.hpp"

class Application;

class Widget
{
	protected:
	Application& m_app;

	private:
	static Application* appPointer;

	bool m_render = true;
	Widget* m_next = nullptr;
	Animator* m_animator = nullptr;

	protected:
	Widget() : m_app(*appPointer) {}

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

class Application : protected AppManager
{
	enum class SeedMode { KEEP, INCREMENT, RANDOMIZE };
	enum class Task { NOTHING, DRAW, RENDER, GENERATE };

	bool m_visRooms;
	bool m_visPaths;
	bool m_visEntrances;

	float m_factor;
	bool m_debugView = false;
	bool m_fullscreen = false;

	GenInput m_input;
	GenOutput m_output;
	Generator m_generator;

	Viewport m_viewport;
	Widget* m_widgetList = nullptr;
	SDL_Texture* m_renderOutput = nullptr;

	Task m_task = Task::GENERATE;
	SeedMode m_seedMode = SeedMode::KEEP;

	Random::seed_type m_seed = 0;
	std::random_device m_randomDevice;

	void Draw();
	void Render();
	bool Update();
	void Generate();
	void LoadDefaults();
	void SetupWidgets();

	void Init(bool full);
	void Quit(bool full);

	template <typename Type>
	Type* GetWidget() const;

	template <typename Type>
	Type& AccessWidget();

	void Schedule(Task task);
	void ScheduleGeneration(SeedMode seedMode);

	public:
	void Run();

	Application() { LoadDefaults(); }
	~Application() { Quit(true); }

	friend class Menu;
	friend class Warning;
};

template <typename Type>
Type* Application::GetWidget() const
{
	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr -> m_next)
	{
		Type* const ptr = dynamic_cast<Type*>(crr);
		if (ptr != nullptr) return ptr;
	}

	return nullptr;
}

template <typename Type>
Type& Application::AccessWidget()
{
	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr -> m_next)
	{
		Type* const ptr = dynamic_cast<Type*>(crr);
		if (ptr != nullptr) return *ptr;
	}

	Widget::appPointer = this;
	Type* const ptr = new Type();

	ptr -> m_animator = dynamic_cast<Animator*>(ptr);
	ptr -> m_next = m_widgetList;

	m_widgetList = ptr;
	return *ptr;
}

inline void Application::Schedule(Task task)
{
	if (m_task < task)
		m_task = task;
}

inline void Application::ScheduleGeneration(SeedMode seedMode)
{
	if (m_task < Task::GENERATE)
	{
		m_task = Task::GENERATE;
		m_seedMode = seedMode;
	}
}