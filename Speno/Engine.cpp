#include "Engine.h"

#include <iostream>
#include <Windows.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include <glm/glm.hpp>

#include "SharedEngineData.h"
#include "Console.h"
#include "Window.h"
#include "UBOmanager.h"
#include "TextureManager.h"
#include "Atmosphere.h"
#include "Camera.h"
#include "Timing.h"
#include "BVH.h"

void Engine::run() {
	int width = SharedEngineData::Instance.window_width;
	int height = SharedEngineData::Instance.window_height;
	ProgramManager::Instance.resizeFramebuffers(width, height);
	Console::Instance.setUseDebugConsole(false);
	glViewport(0, 0, width, height);

	Timing::global.start();

	GLFWwindow* window = Window::Instance.getWindow();
	while (!glfwWindowShouldClose(window)) {
		Timing::global.accumulate();
		Timing::global.restartIfElapsed(1000.0f);

		SharedEngineData::Instance.update();
		Camera::Instance.update();
		UBOmanager::Instance.update();
		
		glfwPollEvents();

		Atmosphere::Instance.render();
		m_renderer.render();
		m_gui.render();
		Window::Instance.swapBuffers();
	}
}

void Engine::initialize() {
#ifdef PUBLIC_RELEASE
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#else
	ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif

	Window::Instance.initialize(this);
	m_renderer.initialize();
	m_gui.initialize();
	
	SharedEngineData::Instance.init();
	TextureManager::Instance.setupSTBN();
	Camera::Instance.init();
	Atmosphere::Instance.init();
	
	glfwSetErrorCallback(errorCallback);
	Window::Instance.setFramebfferSizeCallback(framebufferSizeCallback);
	Window::Instance.setKeyCallback(keyCallback);
}

void Engine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;

		case GLFW_KEY_F11:
			Window::Instance.enable(WINDOW_CHANGE_WINDOW_MODE);
			break;
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_I)) {
				engine->m_gui.toggleInfoWindow();
			}

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
					engine->m_gui.toggleSceneWindow();
				}
				else {
					engine->m_gui.toggleSettingsWindow();
				}
			}

			if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
				engine->m_gui.toggleLogWindow();
			}
		}
	}
}

void Engine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	if (width == 0 || height == 0)
		return;

	SharedEngineData::Instance.window_width = width;
	SharedEngineData::Instance.window_height = height;
	ProgramManager::Instance.resizeFramebuffers(width, height);
	glViewport(0, 0, width, height);
}

void Engine::errorCallback(int error, const char* description) {
	throw std::runtime_error("GLFW Error " + std::to_string(error) + ": " + std::string(description));
}