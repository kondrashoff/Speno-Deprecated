#include "AABB.h"

vec3 AABB::getCenter() {
	return 0.5f * (minimum + maximum);
}

// Volume Cost is faster for BVH
float AABB::getVolumeCost() {
	vec3 extent = maximum - minimum;
	return (extent.x + 1.0f) * (extent.y + 1.0f) * (extent.z + 1.0f);
}

float AABB::getArea() {
	vec3 extent = maximum - minimum;
	return 2.0f * (extent.x * extent.y + extent.y * extent.z + extent.z * extent.x);
}
