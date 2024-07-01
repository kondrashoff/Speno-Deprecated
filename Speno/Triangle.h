#pragma once

#include <glm/glm.hpp>
using namespace glm;

#include "AABB.h"

struct Triangle {
	alignas(16) vec3 v0;
	alignas(16) vec3 v1;
	alignas(16) vec3 v2;

	alignas(16) vec3 n0;
	alignas(16) vec3 n1;
	alignas(16) vec3 n2;

	alignas(8) vec2 uv0;
	alignas(8) vec2 uv1;
	alignas(8) vec2 uv2;

	alignas(4) int material_id;
};