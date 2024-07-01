#include "Window.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "stb_image.h"

#include "FlagManager.h"
#include "SharedEngineData.h"

Window Window::Instance;

void Window::initialize(void* pointer_to_engine) {
	// Try to initialize GLFW
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	FlagSystem::Instance.window = this;
	
	// Create window
	std::string window_title = "Speno Renderer v" + readVersonFromFile();
	m_window = glfwCreateWindow(SharedEngineData::Instance.window_width, SharedEngineData::Instance.window_height, window_title.c_str(), nullptr, nullptr);
	if (!m_window)
		throw std::runtime_error("Failed to create GLFW window");

	// Load and set icon with stb_image
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load("Texture/Icon/icon_48x48.png", &width, &height, &channels, 0);
	if (!data) {
		glfwDestroyWindow(m_window);
		glfwTerminate();
		throw std::runtime_error("Failed to load icon");
	}
	
	GLFWimage icon;
	icon.width = width;
	icon.height = height;
	icon.pixels = data;
	glfwSetWindowIcon(m_window, 1, &icon);
	stbi_image_free(data);

	// Other GLFW settings
	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, pointer_to_engine);
	glfwSetKeyCallback(m_window, defaultKeyCallback);
}

void Window::toggleFullscreen() {
	GLFWmonitor* monitor = glfwGetWindowMonitor(m_window);

	if (monitor == nullptr)
		enterFullscreen();
	else
		enterWindowed();
}

void Window::enterFullscreen() {
	// Save the current window position and size to restore later
	glfwGetWindowPos(m_window, &m_last_pos_x, &m_last_pos_y);
	glfwGetWindowSize(m_window, &m_last_width, &m_last_height);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);

	// Switch to fullscreen
	glfwSetWindowMonitor(m_window, monitor, 0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
}

void Window::enterWindowed() {
	// Restore to windowed mode using the last saved position and size
	glfwSetWindowMonitor(m_window, nullptr, m_last_pos_x, m_last_pos_y, m_last_width, m_last_height, 0);
}

void Window::swapBuffers() {
	glfwSwapBuffers(m_window);
	glfwSwapInterval(isEnabled(WINDOW_VERTICAL_SYNC));

	if (disableIfEnabled(WINDOW_CHANGE_WINDOW_MODE))
	 	toggleFullscreen();

	if (disableIfEnabled(WINDOW_FULLSCREEN))
		enterFullscreen();

	if (disableIfEnabled(WINDOW_WINDOWED))
		enterWindowed();
}

std::string Window::readVersonFromFile() {
	std::ifstream file("version.txt");

	if (!file)
		throw std::runtime_error("Failed to open version file");

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void Window::defaultKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;

		case GLFW_KEY_F11:
			Window::Instance.enable(WINDOW_CHANGE_WINDOW_MODE);
			break;
		}
	}
}

void Window::setKeyCallback(GLFWkeyfun callback) {
	glfwSetKeyCallback(m_window, callback);
}

void Window::setFramebfferSizeCallback(GLFWframebuffersizefun callback) {
	glfwSetFramebufferSizeCallback(m_window, callback);
}

void Window::setShouldClose(bool value) {
	glfwSetWindowShouldClose(m_window, value);
}

void Window::restore() {
	glfwRestoreWindow(m_window);
}

GLFWwindow* Window::getWindow() {
	return m_window;
}

Window::~Window() {
	glfwDestroyWindow(m_window);
}