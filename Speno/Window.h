#pragma once

#include <string>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "FlagManager.h"

class Window : public FlagManager {
public:
	static Window Instance;

	void initialize(void* pointer);
	void setKeyCallback(GLFWkeyfun callback);
	void setFramebfferSizeCallback(GLFWframebuffersizefun callback);
	void setShouldClose(bool value);
	void swapBuffers();
	void restore();

	GLFWwindow* getWindow();
	~Window();

	Window(Window const&) = delete;
	void operator=(Window const&) = delete;

private:
	Window() {}

	void toggleFullscreen();
	void enterFullscreen();
	void enterWindowed();
	std::string readVersonFromFile();

	static void defaultKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	GLFWwindow* m_window;
	int m_last_pos_x = 0;
	int m_last_pos_y = 0;
	int m_last_width = 1280;
	int m_last_height = 720;
};