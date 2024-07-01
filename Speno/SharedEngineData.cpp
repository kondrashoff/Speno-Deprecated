#include "SharedEngineData.h"

#include <GLFW/glfw3.h>

#include "UBOmanager.h"

SharedEngineData SharedEngineData::Instance;

void SharedEngineData::update() {
	double current_time = glfwGetTime();
	float f_current_time = static_cast<float>(current_time);

	delta_time = f_current_time - time;
	time = f_current_time;
	frame++;
}

void SharedEngineData::init() {
	UBOmanager::Instance.create("SharedEngineData", &SharedEngineData::Instance, sizeof(SharedEngineData), GL_DYNAMIC_DRAW);
}

void SharedEngineData::bind(const std::string& program_name) {
	UBOmanager::Instance.bind("SharedEngineData", program_name);
}