#pragma once

#include <string>

#include "AABB.h"

struct UniformMesh {
	int last_node_index;
};

struct Mesh : UniformMesh {
	std::string name;
	int last_triangle_index;
	AABB bounds;
};