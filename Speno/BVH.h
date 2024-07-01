#pragma once

#include <vector>

#include "AABB.h"
#include "Mesh.h"
#include "Triangle.h"

struct PrimitiveReference {
	int primitive_id;
	AABB bounding_box;
	vec3 center;
	uint64_t morton_code;
};

struct BVHNode {
	AABB bounds;
	int index;

	BVHNode() {}

	BVHNode(AABB bounds, int index)
		: bounds(bounds), index(index) {}

	BVHNode(PrimitiveReference pr)
		: bounds(pr.bounding_box), index(-pr.primitive_id - 1) {}

	bool isLeaf() { return index < 0; }
};

struct BVHNodeGPU {
	vec4 data1; // minimum, index
	vec4 data2; // maximum, void

	BVHNodeGPU() {}

	BVHNodeGPU(BVHNode node) {
		data1 = vec4(node.bounds.minimum, intBitsToFloat(node.index));
		data2 = vec4(node.bounds.maximum, 0.0f);
	}
};

class BVH {
public:
	static BVH Instance;

	void buildTLAS(const std::vector<Mesh>& meshes);
	void buildBLAS(Mesh& output_mesh, const std::vector<Triangle>& triangles, int start, int end);

	void setCurrentMeshFilepath(const std::string& filepath);
	void transferToGPU();
	void clear();

	const AABB& getTLASbounds();

	BVH(const BVH&) = delete;
	BVH& operator=(const BVH&) = delete;

private:
	BVHNode ABVH(int start, int end, int depth = 0);
	BVHNode MBVH(int start, int end);
	BVHNode SAHBVH(int start, int end, int last_axis = -1);
	BVHNode LBVH(int start, int end);
	BVHNode LBVHsplitSAH(int start, int end);
	BVHNode LBVHsplit(int start, int end);
	BVHNode LBVHsplitFast(int start, int end);

	float getTreeSahCost();

	void saveBVH(const std::string& filename, size_t number_of_triangles);
	bool loadBVH(const std::string& filename, size_t number_of_triangles);
	
	void buildPrimitivesFrom(const std::vector<Triangle>& triangles, int start, int end);
	void buildPrimitivesFrom(const std::vector<Mesh>& meshes);
	void sortWithAxis(int start, int end, int axis);
	void sortWithMorton(int start, int end);

	uint64_t getMortonCode(dvec3 value);
	uint64_t expandBits(uint64_t value);

	BVH() {}

private:
	std::string m_mesh_filepath;
	std::vector<BVHNode> m_nodes;
	std::vector<PrimitiveReference> m_primitives;
	bool m_is_tlas_builded = false;
	size_t m_first_tlas_node;
	size_t m_first_tlas_primitive;
};