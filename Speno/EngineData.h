#pragma once

struct SharedEngineData {
	float time;
	float delta_time;
	unsigned int frame;

	int mesh_color_method = 1;
	float hdri_luminance_sum = 0.0f;
	alignas(16) glm::vec3 sun_direction = glm::vec3(0.0f, 1.0f, 0.0f);
	alignas(16) glm::vec3 mesh_color = glm::vec3(0.73f);

	alignas(4) bool use_two_sided_geometry = true;
	alignas(4) bool use_temporal_reprojection = false;
	alignas(4) bool use_pathtracing = false;
	alignas(4) bool use_shadows = false;
	alignas(4) bool use_pathtracing_ambient_occlusion = false;
	alignas(4) bool use_hdri = false;
	alignas(4) bool do_not_reproject_movement = false;
};

static SharedEngineData shared_engine_data;
static std::string console_buffer;