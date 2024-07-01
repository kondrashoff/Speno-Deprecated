#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;

#include "FlagManager.h"

struct UniformCamera {
	alignas(16) vec3 lookfrom = vec3(0.0f, 0.0f, 0.0f);
	alignas(16) vec3 right    = vec3(1.0f, 0.0f, 0.0f);
	alignas(16) vec3 up       = vec3(0.0f, 1.0f, 0.0f);
	alignas(16) vec3 front    = vec3(0.0f, 0.0f, 1.0f);
	float tan_half_fov = 0.577f;
};

struct Camera : FlagManager {
	// TODO: Add support for multiple cameras
	static Camera Instance;
	
	vec3 rotations = vec3(0.0f, 0.0f, 0.0f);
	float speed = 3.0f;

	void setPosition(float x, float y, float z);
	void setPosition(const vec3& position);

	void setRotation(float pitch, float yaw, float roll);
	void trackPoint(const vec3& point);
	void stopTracking();

	void update();
	void init();
	void bind(const std::string& program_name);

	UniformCamera getUniformCamera();

private:
	Camera() {}

	void buildViewMatrix();

private:
	GLuint ubo_camera;
	UniformCamera camera[2];

	vec3 velocity = vec3(0.0f);
	vec3 track_point = vec3(0.0f);

	bool is_camera_movement_enabled = false;
	bool is_tracking_point = false;
};