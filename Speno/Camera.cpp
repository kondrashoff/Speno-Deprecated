#include "Camera.h"

#include <iostream>

#include "ProgramManager.h"
#include "UBOmanager.h"
#include "SharedEngineData.h"
#include "Window.h"
#include "BVH.h"

Camera Camera::Instance;

struct AntimationKey {
	vec3 position;
	vec3 rotation;
	float time;
};

//#define USE_ANIMATION

void Camera::update() {
	GLFWwindow* window = Window::Instance.getWindow();

	camera[1] = camera[0];
#ifndef USE_ANIMATION
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
	if (state != GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		is_camera_movement_enabled = false;
		return;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	int centerX = width / 2;
	int centerY = height / 2;

	if (!is_camera_movement_enabled) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPos(window, centerX, centerY);
		is_camera_movement_enabled = true;
		return;
	}

	// Delta time
	float dt = SharedEngineData::Instance.delta_time;

	// Calculate mouse velocity
	double mouse_xpos, mouse_ypos;
	glfwGetCursorPos(window, &mouse_xpos, &mouse_ypos);
	glfwSetCursorPos(window, centerX, centerY);

	float aspect = float(width) / float(height);
	float x_velocity = (centerX - (float)mouse_xpos) / width * aspect * 90.0f;
	float y_velocity = (centerY - (float)mouse_ypos) / height * 90.0f;

	rotations.x += y_velocity;
	rotations.x = clamp(rotations.x, -89.99f, 89.99f);

	rotations.y -= x_velocity;
	rotations.y = mod(rotations.y, 360.0f);

	buildViewMatrix();

	float current_speed = speed * dt;
	vec3 accum_velocity = vec3(0.0f);
	mat3 quat = mat3(camera[0].right, camera[0].up, camera[0].front);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		accum_velocity += quat[2];
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		accum_velocity -= quat[2];
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		accum_velocity -= quat[0];
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		accum_velocity += quat[0];
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		accum_velocity += vec3(0.0f, 1.0f, 0.0f);
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		accum_velocity -= vec3(0.0f, 1.0f, 0.0f);
	}

	float speed_factor = current_speed;
	speed_factor *= glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 0.4 : 1.0;
	speed_factor *= glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 2.7 : 1.0;

	float velocity_length = length(accum_velocity);
	if (velocity_length != 0.0f)
		accum_velocity /= velocity_length;

	if (isEnabled(CAMERA_SMOOTH_MOVEMENT)) {
		camera[0].lookfrom += accum_velocity * speed_factor;
	}
	else camera[0].lookfrom += accum_velocity * speed_factor;
#else
	// Some interpolation animation
	std::vector<AntimationKey> keys;
	float current_time = 15.0 + 0.25 * glfwGetTime();

	{
		AntimationKey key;
		key.position = vec3(-6.5, -1.4, 45.8);
		key.rotation = vec3(359.3, 42.4, 0.0);
		key.time = 20.0;
		keys.push_back(key);

		key.position = vec3(-7.6, 3.1, 41.3);
		key.rotation = vec3(347.8, -47.4, 0.0);
		key.time = 25.0;
		keys.push_back(key);
		key.position = vec3(-7.6, 3.1, 41.3);
		key.rotation = vec3(347.8, -47.4, 0.0);
		key.time = 27.5;
		keys.push_back(key);

		key.position = vec3(-12.2, 1.4, 44.3);
		key.rotation = vec3(211.8, -13.0, 0.0);
		key.time = 32.5;
		keys.push_back(key);
		key.position = vec3(-12.2, 1.4, 44.3);
		key.rotation = vec3(211.8, -13.0, 0.0);
		key.time = 35.0;
		keys.push_back(key);

		key.position = vec3(-9.8, 1.6, 38.7);
		key.rotation = vec3(57.0, -5.2, 0.0);
		key.time = 40.0;
		keys.push_back(key);
		key.position = vec3(-9.8, 1.6, 38.7);
		key.rotation = vec3(57.0, -5.2, 0.0);
		key.time = 42.5;
		keys.push_back(key);

		key.position = vec3(-6.6, 1.6, 37.2);
		key.rotation = vec3(180.3, -3.8, 0.0);
		key.time = 47.5;
		keys.push_back(key);

		key.position = vec3(-6.2, 0.8, 0.3);
		key.rotation = vec3(179.8, -0.4, 0.0);
		key.time = 55.0;
		keys.push_back(key);

		key.position = vec3(-4.5, 1.7, -18.6);
		key.rotation = vec3(213.4, 5.2, 0.0);
		key.time = 60.0;
		keys.push_back(key);

		key.position = vec3(-3.6, 1.7, -17.6);
		key.rotation = vec3(202.9, -0.4, 0.0);
		key.time = 63.0;
		keys.push_back(key);

		key.position = vec3(-1.8, 1.7, -17.5);
		key.rotation = vec3(173.7, -1.4, 0.0);
		key.time = 65.0;
		keys.push_back(key);

		key.position = vec3(1.2, 1.7, -17.0);
		key.rotation = vec3(252.2, -0.8, 0.0);
		key.time = 67.0;
		keys.push_back(key);

		key.position = vec3(4.4, 1.6, -17.4);
		key.rotation = vec3(369.3, -0.8, 0.0);
		key.time = 70.0;
		keys.push_back(key);

		key.position = vec3(5.1, 1.2, 25.5);
		key.rotation = vec3(23.5 + 360.0, -1.6, 0.0);
		key.time = 80.0;
		keys.push_back(key);

		key.position = vec3(5.8, 1.2, 34.1);
		key.rotation = vec3(81.9 + 360.0, -0.4, 0.0);
		key.time = 82.0;
		keys.push_back(key);

		key.position = vec3(5.5, 1.2, 39.1);
		key.rotation = vec3(106.3 + 360.0, 0.8, 0.0);
		key.time = 87.0;
		keys.push_back(key);

		key.position = vec3(4.3, 1.4, 42.0);
		key.rotation = vec3(346.6, 4.2, 0.0);
		key.time = 90.0;
		keys.push_back(key);

		key.position = vec3(-1.0, 1.6, 48.1);
		key.rotation = vec3(202.6, -2.4, 0.0);
		key.time = 100.0;
		keys.push_back(key);
	}

	AntimationKey start_key;
	AntimationKey end_key;

	bool found = false;
	for (int i = 0; i < keys.size(); i++) {
		if (current_time <= keys[i].time) {
			start_key = keys[max(0, i - 1)];
			end_key = keys[i];
			found = true;
			break;
		}
	}

	if (!found) return;

	float interpolation_factor = clamp((current_time - start_key.time) / (end_key.time - start_key.time), 0.0f, 1.0f);
	float smooth_factor = 3.0f * pow(interpolation_factor, 2.0f) - 2.0f * pow(interpolation_factor, 3.0f);
	vec3 interpolated_position = mix(start_key.position, end_key.position, smooth_factor);
	vec3 interpolated_rotation = mix(start_key.rotation, end_key.rotation, smooth_factor);

	camera[0].lookfrom = interpolated_position;
	rotations = interpolated_rotation;
	buildViewMatrix();
#endif
}

void Camera::buildViewMatrix() {
	float pr = radians(rotations.x);
	float sx = sin(pr);
	float cx = cos(pr);

	float yr = radians(rotations.y);
	float sy = sin(yr);
	float cy = cos(yr);

	float rr = radians(rotations.z);
	float sz = sin(rr);
	float cz = cos(rr);

	mat3 rotx = mat3(1.0, 0.0, 0.0, 0.0, cx, -sx, 0.0, sx, cx);
	mat3 roty = mat3(cy, 0.0, sy, 0.0, 1.0, 0.0, -sy, 0.0, cy);
	mat3 rotz = mat3(cz, -sz, 0.0, sz, cz, 0.0, 0.0, 0.0, 1.0);

	mat3 rotation = mat3(cy*cz, -cy*sz, sy, sx*sy*cz+cx*sz, -sx*sy*sz+cx*cz, -sx*cy, -cx*sy*cz+sx*sz, cx*sy*sz+sx*cz, cx*cy);

	camera[0].front = normalize(rotation * vec3(0.0f, 0.0f, 1.0f));
	camera[0].right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera[0].front));
	camera[0].up = normalize(cross(camera[0].front, camera[0].right));
	
	//mat3 rotation = mat3(cy*cz, -cy*sz, sy, sx*sy*cz+cx*sz, -sx*sy*sz+cx*cz, -sx*cy, -cx*sy*cz+sx*sz, cx*sy*sz+sx*cz, cx*cy);

	//camera[0].front = normalize(rotation * vec3(0.0f, 0.0f, 1.0f));
	//camera[0].right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera[0].front));
	//camera[0].up = normalize(cross(camera[0].front, camera[0].right));
}

void Camera::init() {
	FlagSystem::Instance.camera = this;
	enable(CAMERA_SMOOTH_MOVEMENT);

	size_t size = 2 * sizeof(UniformCamera);
	UBOmanager::Instance.create("Cameras", &camera, size, GL_DYNAMIC_DRAW);
}

void Camera::setPosition(float x, float y, float z) {
	camera[0].lookfrom = vec3(x, y, z);
}

void Camera::setPosition(const vec3& position) {
	camera[0].lookfrom = position;
}

void Camera::setRotation(float pitch, float yaw, float roll) {
	rotations = vec3(pitch, yaw, roll);
	buildViewMatrix();
}

void Camera::trackPoint(const vec3& point) {
	track_point = point;
	is_tracking_point = true;
}

void Camera::stopTracking() {
	is_tracking_point = false;
}

void Camera::bind(const std::string& program_name) {
	UBOmanager::Instance.bind("Cameras", program_name);
}

UniformCamera Camera::getUniformCamera() {
	return camera[0];
}