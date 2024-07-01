#pragma once

#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "Texture.h"

class Renderer {
public:
	void initialize();
	void render();
	~Renderer();
};