#pragma once

#include <limits>

#include <glm/glm.hpp>
using namespace glm;

struct AABB {
	vec3 minimum;
	vec3 maximum;
	
	AABB(AABB& a, AABB& b) : minimum(min(a.minimum, b.minimum)), maximum(max(a.maximum, b.maximum)) {}

	AABB(float minx, float miny, float minz, float maxx, float maxy, float maxz) : minimum(vec3(minx, miny, minz)), maximum(vec3(maxx, maxy, maxz)) {}
	AABB(const vec3& minimum, const vec3& maximum) : minimum(minimum), maximum(maximum) {}
	AABB() : minimum(std::numeric_limits<float>::max()), maximum(std::numeric_limits<float>::lowest()) {}

	vec3 getCenter();
	float getVolumeCost();
	float getArea();
};