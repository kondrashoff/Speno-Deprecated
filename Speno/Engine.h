#pragma once

#include "Renderer.h"
#include "ProgramManager.h"
#include "GUI.h"

class Engine {
public:
	void run();
	void initialize();

private:
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void errorCallback(int error, const char* description);

private:
	Renderer m_renderer;
	GUI m_gui;
};