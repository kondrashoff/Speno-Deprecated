#pragma once

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
using namespace glm;

struct SharedEngineData {
	static SharedEngineData Instance;

	int frame = 0;
	float time = 0.0f;
	float delta_time = 0.0f;

	// (1920, 1080), (1600, 900), (1280, 720), (1024, 576), (800, 450), (640, 360), (480, 270)
	int window_width = 1280;
	int window_height = 720;

	int max_depth = 2;

	void init();
	void update();
	void bind(const std::string& program_name);

	SharedEngineData(SharedEngineData const&) = delete;
	void operator=(SharedEngineData const&) = delete;

private:
	SharedEngineData() {}
};