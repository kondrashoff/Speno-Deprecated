#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
using namespace ImGui;

#include "Window.h"

class GUI {
public:
	void initialize();
	void render();

	void toggleInfoWindow();
	void toggleSettingsWindow();
	void toggleSceneWindow();
	void toggleLogWindow();

	~GUI();

private:
	void draw();
	void drawMainMenuBar();
	void drawInfoWindow();
	void drawSettingsWindow();
	void drawSceneWindow();
	void drawLogWindow();

private:
	ImVec4 TOOLTIP_COLOR_IMPORTANT = ImVec4(0.912f, 0.914f, 0.920f, 1.0f);
	ImVec4 TOOLTIP_COLOR_BASIC = ImVec4(0.793f, 0.793f, 0.664f, 1.0f);
	ImVec4 TOOLTIP_COLOR_WARNING = ImVec4(0.718f, 0.483f, 0.0f, 1.0f);
	ImVec4 TOOLTIP_COLOR_DANGER = ImVec4(0.644f, 0.003f, 0.005f, 1.0f);

	bool m_is_info_window_open = false;
	bool m_is_settings_window_open = false;
	bool m_is_scene_window_open = false;
	bool m_is_log_window_open = false;
};