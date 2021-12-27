#pragma once
#include "utils.hpp"

class Text
{
	int m_width = 0;
	int m_height = 0;

	SDL_Texture* m_texture = nullptr;

	public:
	Text() = default;
	~Text() { Clear(); }

	Text(const Text& ref) = delete;
	Text& operator=(const Text& ref) = delete;

	Text(Text&& ref) noexcept;
	Text& operator=(Text&& ref) noexcept;

	void Clear();
	Vec GetSize() const;
	SDL_Texture* GetTexture() const;

	friend class AppManager;
};

class AppManager
{
	int m_width = 0;
	int m_height = 0;

	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	std::map<int, TTF_Font*> m_fonts;

	public:
	AppManager();
	~AppManager();

	AppManager(const AppManager& ref) = delete;
	AppManager& operator=(const AppManager& ref) = delete;

	AppManager(AppManager&& ref) noexcept = delete;
	AppManager& operator=(AppManager&& ref) noexcept = delete;

	Vec GetSize() const;
	SDL_Window* GetWindow() const;
	SDL_Renderer* GetRenderer() const;

	void UnloadAllFonts();
	void UnloadFont(int id);
	void LoadFont(int id, int size, const std::string& path);
	void LoadFont(int id, int size, const void* data, int dataSize);

	void DestroyWindow();
	void CreateWindow(const std::string& title, int width, int height, Uint32 flags = 0);

	Text RenderText(const std::string& str, int id, int style = TTF_STYLE_NORMAL, const SDL_Color& color = { 0xFF, 0xFF, 0xFF, 0xFF }) const;
};

inline Vec Text::GetSize() const
{
	return Vec(m_width, m_height);
}

inline SDL_Texture* Text::GetTexture() const
{
	return m_texture;
}

inline Vec AppManager::GetSize() const
{
	return Vec(m_width, m_height);
}

inline SDL_Window* AppManager::GetWindow() const
{
	return m_window;
}

inline SDL_Renderer* AppManager::GetRenderer() const
{
	return m_renderer;
}