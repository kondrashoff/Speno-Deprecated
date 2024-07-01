#include "Renderer.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Quad.h"
#include "ProgramManager.h"
#include "SharedEngineData.h"
#include "Console.h"

void Renderer::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	ProgramManager::Instance.drawInOrder();

#ifndef PUBLIC_RELEASE
	GLenum error = glGetError();

	if (error != GL_NO_ERROR) {
		Console::Instance.push("GL Error: " + error);
	}
#endif
}

void Renderer::initialize() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to initialize GLAD.");
	
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	Quad::Instance.initialize();
}

Renderer::~Renderer() {
	glfwTerminate();
}