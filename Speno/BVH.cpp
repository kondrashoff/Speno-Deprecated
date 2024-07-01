#include "BVH.h"

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include "SSBOmanager.h"

BVH BVH::Instance;

void BVH::buildTLAS(const std::vector<Mesh>& meshes) {
	if (m_nodes.empty())
		throw std::runtime_error("There is no BLAS to build TLAS");

	if (meshes.empty()) 
		throw std::runtime_error("There is no meshes to build TLAS");

	if (m_is_tlas_builded) {
		m_primitives.erase(m_primitives.begin() + m_first_tlas_primitive, m_primitives.end());
		m_nodes.erase(m_nodes.begin() + m_first_tlas_node, m_nodes.end());
	}

	m_is_tlas_builded = true;
	m_first_tlas_node = m_nodes.size();
	m_first_tlas_primitive = m_primitives.size();

	int start = m_primitives.size();
	buildPrimitivesFrom(meshes);
	int end = m_primitives.size();

	BVHNode root = SAHBVH(start, end);
	m_nodes.push_back(root);
}

void BVH::buildBLAS(Mesh& output_mesh, const std::vector<Triangle>& triangles, int start, int end) {
	if (triangles.empty())
		throw std::runtime_error("There is no triangles to build BLAS");

	if (m_is_tlas_builded) {
		m_primitives.erase(m_primitives.begin() + m_first_tlas_primitive, m_primitives.end());
		m_nodes.erase(m_nodes.begin() + m_first_tlas_node, m_nodes.end());
		m_is_tlas_builded = false;
	}

	size_t number_of_triangles = end - start;

	size_t start_index = m_mesh_filepath.find_last_of('/');
	if(start_index == std::string::npos) start_index = m_mesh_filepath.find_last_of('\\');
	start_index++;

	size_t end_index = m_mesh_filepath.find_last_of('.');

	std::string meshname = m_mesh_filepath.substr(start_index, end_index - start_index);
	std::string bvhname = meshname + "_" + output_mesh.name + ".blas";

	if (loadBVH(bvhname, number_of_triangles)) {
		output_mesh.last_node_index = m_nodes.size() - 1;
		output_mesh.bounds = m_nodes[output_mesh.last_node_index].bounds;
		return;
	}

	buildPrimitivesFrom(triangles, start, end);

	BVHNode root = SAHBVH(start, end);
	m_nodes.push_back(root);

	//saveBVH(bvhname, number_of_triangles);

	output_mesh.last_node_index = m_nodes.size() - 1;
	output_mesh.bounds = root.bounds;
}

BVHNode BVH::ABVH(int start, int end, int depth) {
	int num_prims = end - start;

	if (num_prims == 1) return BVHNode(m_primitives[start]);

	sortWithAxis(start, end, depth % 3);

	int mid = start + num_prims / 2;

	BVHNode left_child = ABVH(start, mid, depth + 1);
	BVHNode right_child = ABVH(mid, end, depth + 1);

	int index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(left_child);
	m_nodes.push_back(right_child);

	AABB bounds = AABB(left_child.bounds, right_child.bounds);

	return BVHNode(bounds, index);
}

BVHNode BVH::MBVH(int start, int end) {
	int num_prims = end - start;

	if (num_prims == 1) return BVHNode(m_primitives[start]);

	AABB node = m_primitives[start].bounding_box;
	for (int i = start + 1; i < end; i++)
		node = AABB(node, m_primitives[i].bounding_box);

	vec3 size = node.maximum - node.minimum;

	if (size.x > size.y && size.x > size.z) sortWithAxis(start, end, 0);
	else if (size.y > size.z) sortWithAxis(start, end, 1);
	else sortWithAxis(start, end, 2);

	int mid = start + num_prims / 2;

	BVHNode left_child = MBVH(start, mid);
	BVHNode right_child = MBVH(mid, end);

	int index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(left_child);
	m_nodes.push_back(right_child);

	return BVHNode(node, index);
}

BVHNode BVH::SAHBVH(int start, int end, int last_axis) {
	int num_prims = end - start;

	if (num_prims == 1) return BVHNode(m_primitives[start]);

	if (num_prims == 2) {
		BVHNode left_child = BVHNode(m_primitives[start]);
		BVHNode right_child = BVHNode(m_primitives[start + 1]);

		AABB bounds = AABB(left_child.bounds, right_child.bounds);

		int index = (int)m_nodes.size();
		m_nodes.push_back(left_child);
		m_nodes.push_back(right_child);

		return BVHNode(bounds, index);
	}

	// Find the best split
	int best_split = -1;
	int best_axis = -1;
	double best_cost = std::numeric_limits<double>::max();

	if (last_axis != -1) {
		std::vector<AABB> rights(num_prims);

		AABB sum = m_primitives[end - 1].bounding_box;
		for (int i = num_prims - 1; i >= 0; i--) {
			rights[i] = sum;

			if (i > 0) {
				sum = AABB(sum, m_primitives[start + i - 1].bounding_box);
			}
		}

		AABB left;
		for (int i = start + 1; i < end; i++) {
			left = AABB(left, m_primitives[i - 1].bounding_box);
			AABB& right = rights[i - start];

			double left_count = static_cast<double>(i - start);
			double right_count = static_cast<double>(end - i);

			double left_area = static_cast<double>(left.getVolumeCost());
			double right_area = static_cast<double>(right.getVolumeCost());

			double cost = left_area * left_count + right_area * right_count;

			if (cost < best_cost) {
				best_cost = cost;
				best_axis = last_axis;
				best_split = i;
			}
		}
	}

	for (int axis = 0; axis < 3; axis++) {
		if (axis == last_axis) continue;

		// Sort along this axis
		sortWithAxis(start, end, axis);

		std::vector<AABB> rights(num_prims);

		AABB sum = m_primitives[end - 1].bounding_box;
		for (int i = num_prims - 1; i >= 0; i--) {
			rights[i] = sum;

			if (i > 0) {
				sum = AABB(sum, m_primitives[start + i - 1].bounding_box);
			}
		}

		AABB left;
		for (int i = start + 1; i < end; i++) {
			left = AABB(left, m_primitives[i - 1].bounding_box);
			AABB& right = rights[i - start];

			double left_count = static_cast<double>(i - start);
			double right_count = static_cast<double>(end - i);

			double left_area = static_cast<double>(left.getVolumeCost());
			double right_area = static_cast<double>(right.getVolumeCost());

			double cost = left_area * left_count + right_area * right_count;
			
			if (cost < best_cost) {
				best_cost = cost;
				best_axis = axis;
				best_split = i;
			}
		}
	}

	// Sort primitives on best axis
	if(best_axis != 2 || last_axis == 2) sortWithAxis(start, end, best_axis);

	BVHNode left_child = SAHBVH(start, best_split, best_axis);
	BVHNode right_child = SAHBVH(best_split, end, best_axis);

	int index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(left_child);
	m_nodes.push_back(right_child);

	AABB bounds = AABB(left_child.bounds, right_child.bounds);

	return BVHNode(bounds, index);
}

BVHNode BVH::LBVH(int start, int end) {
	AABB whole = m_primitives[start].bounding_box;
	for (int i = start + 1; i < end; i++)
		whole = AABB(whole, m_primitives[i].bounding_box);

	dvec3 whole_size = dvec3(whole.maximum) - dvec3(whole.minimum);

	for (int i = start; i < end; i++) {
		dvec3 center = m_primitives[i].center;

		center -= dvec3(whole.minimum);
		center /= whole_size;

		m_primitives[i].morton_code = getMortonCode(center);
	}

	sortWithMorton(start, end);
	return LBVHsplitSAH(start, end);
}

BVHNode BVH::LBVHsplitSAH(int start, int end) {
	int num_prims = end - start;

	if (num_prims == 1) return BVHNode(m_primitives[start]);

	if (num_prims == 2) {
		BVHNode left_child = BVHNode(m_primitives[start]);
		BVHNode right_child = BVHNode(m_primitives[start + 1]);

		AABB bounds = AABB(left_child.bounds, right_child.bounds);

		int index = static_cast<int>(m_nodes.size());
		m_nodes.push_back(left_child);
		m_nodes.push_back(right_child);

		return BVHNode(bounds, index);
	}

	int best_split = -1;
	double best_cost = std::numeric_limits<double>::max();

	std::vector<AABB> rights(num_prims);

	AABB sum = m_primitives[end - 1].bounding_box;
	for (int i = num_prims - 1; i >= 0; i--) {
		rights[i] = sum;

		if (i > 0) {
			sum = AABB(sum, m_primitives[start + i - 1].bounding_box);
		}
	}

	AABB left;
	for (int i = start + 1; i < end; i++) {
		left = AABB(left, m_primitives[i - 1].bounding_box);
		AABB right = rights[i - start];

		double left_count = static_cast<double>(i - start);
		double right_count = static_cast<double>(end - i);

		double left_area = static_cast<double>(left.getArea());
		double right_area = static_cast<double>(right.getArea());

		double cost = left_area * left_count + right_area * right_count;

		if (cost < best_cost) {
			best_cost = cost;
			best_split = i;
		}
	}

	BVHNode left_child = LBVHsplitSAH(start, best_split);
	BVHNode right_child = LBVHsplitSAH(best_split, end);

	int index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(left_child);
	m_nodes.push_back(right_child);

	AABB bounds = AABB(left_child.bounds, right_child.bounds);

	return BVHNode(bounds, index);
}

BVHNode BVH::LBVHsplit(int start, int end) {
	int num_prims = end - start;

	if (num_prims == 1) return BVHNode(m_primitives[start]);

	int best_split = start + num_prims / 2;
	uint64_t best_distance = 0u;

	for (int i = start + 1; i < end; i++) {
		uint64_t left = m_primitives[i - 1].morton_code;
		uint64_t right = m_primitives[i].morton_code;

		uint64_t distance = right > left ? right - left : left - right;

		if (distance > best_distance) {
			best_distance = distance;
			best_split = i;
		}
	}

	BVHNode left_child = LBVHsplit(start, best_split);
	BVHNode right_child = LBVHsplit(best_split, end);

	int index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(left_child);
	m_nodes.push_back(right_child);

	AABB bounds = AABB(left_child.bounds, right_child.bounds);

	return BVHNode(bounds, index);
}

BVHNode BVH::LBVHsplitFast(int start, int end) {
	int num_prims = end - start;

	if (num_prims == 1) return BVHNode(m_primitives[start]);

	int mid = start + num_prims / 2;

	BVHNode left_child = LBVHsplitFast(start, mid);
	BVHNode right_child = LBVHsplitFast(mid, end);

	int index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(left_child);
	m_nodes.push_back(right_child);

	AABB bounds = AABB(left_child.bounds, right_child.bounds);

	return BVHNode(bounds, index);
}

float BVH::getTreeSahCost() {
	const float sah_cost_node = 2.0;
	const float sah_cost_leaf = 5.5;

	float sah_sum_node = 0.0f;
	float sah_sum_leaf = 0.0f;

	for (BVHNode& node : m_nodes) {
		if(node.isLeaf()) sah_sum_leaf += node.bounds.getArea();
		else sah_sum_node += node.bounds.getArea();
	}

	float root_area = m_nodes[0].bounds.getArea();
	float node_cost = sah_cost_node * sah_sum_node;
	float leaf_cost = sah_cost_leaf * sah_sum_leaf;

	return (node_cost + leaf_cost) / root_area;
}

void BVH::saveBVH(const std::string& filename, size_t number_of_triangles) {
	size_t slash_index = m_mesh_filepath.find_last_of('/');
	if (slash_index == std::string::npos) slash_index = m_mesh_filepath.find_last_of('\\');

	std::string path = m_mesh_filepath.substr(0, slash_index + 1) + "BVH";
	std::filesystem::create_directories(path);
	
	size_t num_nodes = m_nodes.size();
	std::ofstream outfile(path + "/" + filename, std::ios::binary);

	outfile.write(reinterpret_cast<const char*>(&number_of_triangles), sizeof(size_t));
	outfile.write(reinterpret_cast<const char*>(&num_nodes), sizeof(size_t));

	for (const BVHNode& node : m_nodes) {
		outfile.write(reinterpret_cast<const char*>(&node), sizeof(BVHNode));
	}

	outfile.close();
}

bool BVH::loadBVH(const std::string& filename, size_t number_of_triangles) {
	size_t slash_index = m_mesh_filepath.find_last_of('/');
	if (slash_index == std::string::npos) slash_index = m_mesh_filepath.find_last_of('\\');

	std::string path = m_mesh_filepath.substr(0, slash_index + 1);
	std::ifstream infile(path + "BVH/" + filename, std::ios::binary);

	if (!infile.is_open()) return false;

	size_t num_triangles;
	size_t num_nodes;

	infile.read(reinterpret_cast<char*>(&num_triangles), sizeof(size_t));
	infile.read(reinterpret_cast<char*>(&num_nodes), sizeof(size_t));

	if (num_triangles != number_of_triangles) return false;

	m_nodes.reserve(num_nodes);

	for (size_t i = 0; i < num_nodes; ++i) {
		BVHNode node;
		infile.read(reinterpret_cast<char*>(&node), sizeof(BVHNode));
		m_nodes.push_back(node);
	}

	infile.close();
	return true;
}

uint64_t BVH::getMortonCode(dvec3 value) {
	//const double resolution = 1023.0; // 3 * 10 bits
	const double resolution = 1048575.0; // 3 * 20 bits

	double x = value.x;
	double y = value.y;
	double z = value.z;
	
	x = fmin(fmax(x * resolution, 0.0), resolution);
	y = fmin(fmax(y * resolution, 0.0), resolution);
	z = fmin(fmax(z * resolution, 0.0), resolution);

	const uint64_t xx = expandBits(static_cast<uint64_t>(x));
	const uint64_t yy = expandBits(static_cast<uint64_t>(y));
	const uint64_t zz = expandBits(static_cast<uint64_t>(z));

	return xx * 4u + yy * 2u + zz;
}

// for 64 bit lbvh
uint64_t BVH::expandBits(uint64_t value) {
	value = (value * 0x000100000001u) & 0xFFFF00000000FFFFu;
	value = (value * 0x000000010001u) & 0x00FF0000FF0000FFu;
	value = (value * 0x000000000101u) & 0xF00F00F00F00F00Fu;
	value = (value * 0x000000000011u) & 0x30C30C30C30C30C3u;
	value = (value * 0x000000000005u) & 0x9249249249249249u;

	return value;
}

// for 32 bit lbvh
/*uint64_t BVH::expandBits(uint64_t value) {
	value = (value * 0x00010001u) & 0xFF0000FFu;
	value = (value * 0x00000101u) & 0x0F00F00Fu;
	value = (value * 0x00000011u) & 0xC30C30C3u;
	value = (value * 0x00000005u) & 0x49249249u;
	return value;
}*/

void BVH::transferToGPU() {
	size_t size = m_nodes.size() * sizeof(BVHNodeGPU);
	BVHNodeGPU* data = new BVHNodeGPU[m_nodes.size()];

	std::transform(m_nodes.begin(), m_nodes.end(), data,
		[](const BVHNode& node) { return BVHNodeGPU(node); }
	);

	SSBOmanager::Instance.createOrSet(SSBO_BVH_BINDING, data, size);
	delete[] data;
}

void BVH::buildPrimitivesFrom(const std::vector<Triangle>& triangles, int start, int end) {
	for (int i = start; i < end; i++) {
		PrimitiveReference pr;

		Triangle tri = triangles[i];
		pr.primitive_id = static_cast<int>(i);
		pr.bounding_box.minimum = min(min(tri.v0, tri.v1), tri.v2);
		pr.bounding_box.maximum = max(max(tri.v0, tri.v1), tri.v2);
		pr.center = pr.bounding_box.getCenter();
		//pr.center = (tri.v0 + tri.v1 + tri.v2) / 3.0f; // Somehow center of triangle is worse than center of aabb

		m_primitives.push_back(pr);
	}
}

void BVH::buildPrimitivesFrom(const std::vector<Mesh>& meshes) {
	for (int i = 0; i < meshes.size(); i++) {
		PrimitiveReference pr;

		Mesh mesh = meshes[i];
		pr.primitive_id = static_cast<int>(i);
		pr.bounding_box = mesh.bounds;
		pr.center = mesh.bounds.getCenter();

		m_primitives.push_back(pr);
	}
}

void BVH::sortWithMorton(int start, int end) {
	std::sort(m_primitives.begin() + start, m_primitives.begin() + end,
		[](const PrimitiveReference& a, const PrimitiveReference& b) {
			return a.morton_code < b.morton_code;
		}
	);
}

void BVH::sortWithAxis(int start, int end, int axis) {
	std::sort(m_primitives.begin() + start, m_primitives.begin() + end,
		[axis](const PrimitiveReference& a, const PrimitiveReference& b) {
			return a.center[axis] < b.center[axis];
		}
	);
}

void BVH::clear() {
	m_is_tlas_builded = false;
	m_mesh_filepath.clear();
	m_primitives.clear();
	m_nodes.clear();
}

const AABB& BVH::getTLASbounds() {
	if (m_nodes.size() == 0) return AABB();
	return m_nodes.back().bounds;
}

void BVH::setCurrentMeshFilepath(const std::string& filepath) {
	m_mesh_filepath = filepath;
}