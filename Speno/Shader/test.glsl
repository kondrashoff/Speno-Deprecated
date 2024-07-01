#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 fragColor;
in vec2 texCoords;

uniform vec2 resolution;

#include "defines.glsl"
#include "shared_engine_data.glsl"
#include "random.glsl"
#include "camera.glsl"
#include "hit.glsl"
#include "scene.glsl"

bool intersectTriangle(inout HitData hit, Ray ray, int triangle_index) {
	Triangle triangle = triangles[triangle_index];
	vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
	vec3 e2 = triangle.vertices[2] - triangle.vertices[0];
	vec3 pvec = cross(ray.direction, e2);
	float det = dot(e1, pvec);

	float inv_det = 1.0 / det;
	vec3 tvec = ray.origin - triangle.vertices[0];

	float u = inv_det * dot(tvec, pvec);
	if (u < 0.0 || u > 1.0) return false;

	vec3 qvec = cross(tvec, e1);
	float v = inv_det * dot(ray.direction, qvec);
	if (v < 0.0) return false;

	float UpV = u + v;
	if (UpV > 1.0) return false;

	float t = dot(e2, qvec) * inv_det;
	if (t < 0.0 || t > hit.t) return false;

	hit.t = t;
	hit.was_intersection = true;

	return true;
}

vec2 testAABB(HitData hit, Ray ray, AABB aabb) {
	vec3 n = (aabb.minimum - ray.origin) * ray.inverse_direction;
	vec3 f = (aabb.maximum - ray.origin) * ray.inverse_direction;

	vec3 tmin = min(f, n);
	vec3 tmax = max(f, n);

	float t0 = max(tmin.x, max(tmin.y, tmin.z));
	float t1 = min(tmax.x, min(tmax.y, tmax.z));

	if (t1 < t0 || t0 > hit.t) return vec2(-1.0);

	return vec2(t0, t1);
}

bool traverseBLAS(int root_index, inout HitData hit, Ray ray) {
	BVHNode node = bvh_nodes[root_index];
	int stack[BLAS_STACK_SIZE];
	int ptr = 0;

	while (true) {
		int leftIndex = node.left_id;
		int rightIndex = node.right_id;

		ptr = min(BLAS_STACK_SIZE - 1, ptr);

		if (rightIndex != -1) {
			BVHNode left = bvh_nodes[leftIndex];
			BVHNode right = bvh_nodes[rightIndex];

			vec2 leftHit = testAABB(hit, ray, left.aabb);
			vec2 rightHit = testAABB(hit, ray, right.aabb);

			if (leftHit.y > 0.0) {
				if (rightHit.y > 0.0) {
					float leftCenter = leftHit.x + leftHit.y;
					float rightCenter = rightHit.x + rightHit.y;

					if (leftCenter > rightCenter) {
						node = right;
						stack[ptr++] = leftIndex;
						continue;
					}

					node = left;
					stack[ptr++] = rightIndex;
					continue;
				}

				node = left;
				continue;
			}
			else if (rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else intersectTriangle(hit, ray, leftIndex);

		if (ptr != 0) node = bvh_nodes[stack[--ptr]];
		else return hit.was_intersection;
	}
}

bool traverse(inout HitData hit, Ray ray) {
	hit = HitData();
	ray.inverse_direction = 1.0 / ray.direction;

	BVHNode node = bvh_nodes[bvh_nodes.length() - 1];
	int stack[TLAS_STACK_SIZE];
	int ptr = 0;

	while (true) {
		int leftIndex = node.left_id;
		int rightIndex = node.right_id;

		ptr = min(TLAS_STACK_SIZE - 1, ptr);

		if (rightIndex != -1) {
			BVHNode left = bvh_nodes[leftIndex];
			BVHNode right = bvh_nodes[rightIndex];

			vec2 leftHit = testAABB(hit, ray, left.aabb);
			vec2 rightHit = testAABB(hit, ray, right.aabb);

			if (leftHit.y > 0.0) {
				if (rightHit.y > 0.0) {
					float leftCenter = leftHit.x + leftHit.y;
					float rightCenter = rightHit.x + rightHit.y;

					if (leftCenter > rightCenter) {
						node = right;
						stack[ptr++] = leftIndex;
						continue;
					}

					node = left;
					stack[ptr++] = rightIndex;
					continue;
				}

				node = left;
				continue;
			}
			else if (rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else {
			Mesh mesh = meshes[leftIndex];
			int root_index = mesh.last_node_index;
			traverseBLAS(root_index, hit, ray);
		}

		if (ptr != 0) node = bvh_nodes[stack[--ptr]];
		else return hit.was_intersection;
	}
}

bool traverseBLASShadow(int root_index, inout HitData hit, Ray ray) {
	BVHNode node = bvh_nodes[root_index];
	int stack[BLAS_STACK_SIZE];
	int ptr = 0;

	while (true) {
		int leftIndex = node.left_id;
		int rightIndex = node.right_id;

		ptr = min(BLAS_STACK_SIZE - 1, ptr);

		if (rightIndex != -1) {
			BVHNode left = bvh_nodes[leftIndex];
			BVHNode right = bvh_nodes[rightIndex];

			vec2 leftHit = testAABB(hit, ray, left.aabb);
			vec2 rightHit = testAABB(hit, ray, right.aabb);

			if (leftHit.y > 0.0) {
				node = left;
				if (rightHit.y > 0.0) stack[ptr++] = rightIndex;
			}
			else if (rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else if (intersectTriangle(hit, ray, leftIndex)) return true;

		if (ptr != 0) node = bvh_nodes[stack[--ptr]];
		else return hit.was_intersection;
	}
}

bool traverseShadow(inout HitData hit, Ray ray) {
	hit = HitData();
	ray.inverse_direction = 1.0 / ray.direction;

	BVHNode node = bvh_nodes[bvh_nodes.length() - 1];
	int stack[TLAS_STACK_SIZE];
	int ptr = 0;

	while (true) {
		int leftIndex = node.left_id;
		int rightIndex = node.right_id;

		ptr = min(TLAS_STACK_SIZE - 1, ptr);

		if (rightIndex != -1) {
			BVHNode left = bvh_nodes[leftIndex];
			BVHNode right = bvh_nodes[rightIndex];

			vec2 leftHit = testAABB(hit, ray, left.aabb);
			vec2 rightHit = testAABB(hit, ray, right.aabb);

			if (leftHit.y > 0.0) {
				node = left;
				if (rightHit.y > 0.0) stack[ptr++] = rightIndex;
			}
			else if (rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else {
			Mesh mesh = meshes[leftIndex];
			int root_index = mesh.last_node_index;
			if (traverseBLAS(root_index, hit, ray)) return true;
		}

		if (ptr != 0) node = bvh_nodes[stack[--ptr]];
		else return hit.was_intersection;
	}
}

vec3 getRandomOnSphere() {
	vec2 r = vec2(randomFloatHQ(), randomFloatHQ());

	float theta = TAU * r.x;
	float phi = acos(2.0 * r.y - 1.0);

	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	return vec3(x, y, z);
}

void main() {
	setupHQRandomSeed();

	vec2 uv = vec2(1.0 - texCoords.x, texCoords.y);
	Ray ray = getRay(uv);

	HitData hit = HitData();
	traverse(hit, ray);

	AABB root = bvh_nodes[bvh_nodes.length() - 1].aabb;
	float size = length(root.maximum - root.minimum);

	if (!hit.was_intersection) {
		fragColor = vec4(0.7, 0.8, 1.0, 1.0);
	}
	else fragColor = vec4(1.0);

	float r = randomFloatHQ();
	float density = 1.0 / size;
	float neg_inv_density = -1.0 / density;
	float rand_dist = hit.was_intersection ? hit.t * r : neg_inv_density * log(r);

	ray.origin += ray.direction * rand_dist;
	ray.direction = normalize(vec3(1));

	if (traverseShadow(hit, ray)) {
		fragColor *= vec4(INV_PI);
		return;
	}

	/*ray.origin += ray.direction * (hit.t - 0.04);
	ray.direction = getRandomOnSphere();

	if (traverseShadow(hit, ray)) {
		fragColor = vec4(0.17);
		return;
	}

	fragColor = vec4(0.73);*/
}