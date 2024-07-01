#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 heatColor;
in vec2 texCoords;

uniform vec2 resolution;

#include "defines.glsl"
#include "camera.glsl"
#include "scene.glsl"
#include "wavelength.glsl"
#include "hit.glsl"

uint intersectTriangle(inout HitData hit, Ray ray, int triangle_index) {
	Triangle triangle = triangles[triangle_index];
    vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e2 = triangle.vertices[2] - triangle.vertices[0];
    vec3 pvec = cross(ray.direction, e2);
    float det = dot(e1, pvec);

    float inv_det = 1.0 / det;
    vec3 tvec = ray.origin - triangle.vertices[0];

    float u = inv_det * dot(tvec, pvec);
    if(u < 0.0 || u > 1.0) return 4u;

    vec3 qvec = cross(tvec, e1);
    float v = inv_det * dot(ray.direction, qvec);
    if(v < 0.0) return 5u;

	float UpV = u + v;
	if(UpV > 1.0) return 5u;

    float t = dot(e2, qvec) * inv_det;
    if(t < 0.0 || t > hit.t) return 6u;

    hit.t = t;
    hit.was_intersection = true;

    return 8u;
}

vec2 testAABB(HitData hit, Ray ray, vec3 minimum, vec3 maximum) {
	vec3 n = (minimum - ray.origin) * ray.inverse_direction;
	vec3 f = (maximum - ray.origin) * ray.inverse_direction;

	vec3 tmin = min(f, n);
	vec3 tmax = max(f, n);

	float t0 = max(tmin.x, max(tmin.y, tmin.z));
	float t1 = min(tmax.x, min(tmax.y, tmax.z));

	if (t1 < t0 || t0 > hit.t) return vec2(-1.0);

	return vec2(t0, t1);
}

uint traverseBLAS(int root_index, inout HitData hit, Ray ray) {
	BVHNode node = bvh_nodes[root_index];
    int stack[BLAS_STACK_SIZE];
    uint ptr = 0;

    uint attempts_counter = 0u;

	while(true) {
		ptr = min(BLAS_STACK_SIZE - 1, ptr);
		int index = floatBitsToInt(node.data1.w);

		if(index > -1) {
			BVHNode left = bvh_nodes[index];
			BVHNode right = bvh_nodes[index + 1];

			vec2 leftHit = testAABB(hit, ray, left.data1.xyz, left.data2.xyz);
            vec2 rightHit = testAABB(hit, ray, right.data1.xyz, right.data2.xyz);

            attempts_counter += 2u;

			if(leftHit.y > 0.0 && rightHit.y > 0.0) {
				float leftCenter = leftHit.x + leftHit.y;
				float rightCenter = rightHit.x + rightHit.y;

				if (leftCenter > rightCenter) {
					stack[ptr++] = index;
					node = right;
					continue;
				}

				stack[ptr++] = index + 1;
				node = left;
				continue;
			}
			else if(leftHit.y > 0.0) {
				node = left;
				continue;
			}
			else if(rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else attempts_counter += intersectTriangle(hit, ray, -index - 1);

		if(ptr != 0u) node = bvh_nodes[stack[--ptr]];
		else return attempts_counter;
	}
}

uint traverse(inout HitData hit, Ray ray) {
	hit = HitData();
	ray.inverse_direction = 1.0 / ray.direction;

	int num_nodes = bvh_nodes.length();
	if (num_nodes == 0) return 0;

	BVHNode node = bvh_nodes[num_nodes - 1];
	int stack[TLAS_STACK_SIZE];
    uint ptr = 0;

    uint attempts_counter = 0;

	while(true) {
		ptr = min(TLAS_STACK_SIZE - 1, ptr);
		int index = floatBitsToInt(node.data1.w);

		if(index > -1) {
			BVHNode left = bvh_nodes[index];
			BVHNode right = bvh_nodes[index + 1];

			vec2 leftHit = testAABB(hit, ray, left.data1.xyz, left.data2.xyz);
            vec2 rightHit = testAABB(hit, ray, right.data1.xyz, right.data2.xyz);

            attempts_counter += 2u;

			if(leftHit.y > 0.0 && rightHit.y > 0.0) {
				float leftCenter = leftHit.x + leftHit.y;
				float rightCenter = rightHit.x + rightHit.y;

				if (leftCenter > rightCenter) {
					stack[ptr++] = index;
					node = right;
					continue;
				}

				stack[ptr++] = index + 1;
				node = left;
				continue;
			}
			else if(leftHit.y > 0.0) {
				node = left;
				continue;
			}
			else if(rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else {
			Mesh mesh = meshes[-index - 1];
			int root_index = mesh.last_node_index;
			attempts_counter += traverseBLAS(root_index, hit, ray);
		}

		if(ptr != 0u) node = bvh_nodes[stack[--ptr]];
		else return attempts_counter;
	}
}

void main() {
	vec2 uv = vec2(1.0 - texCoords.x, texCoords.y);
	Ray ray = getRay(uv);
	HitData hit;
	
    float attempts = float(traverse(hit, ray));

	float heat_level = 380.0;
	if(attempts < 64.0) { // > 60 fps (purple)
		heat_level = mix(380.0, 440.0, attempts / 64.0);
	}
	else if(attempts < 128.0) { // > 30 fps (blue)
		heat_level = mix(440.0, 490.0, (attempts - 64.0) / 64.0);
	}
	else if(attempts < 191.0) { // > 20 fps (green)
		heat_level = mix(490.0, 510.0, (attempts - 128.0) / 63.0);
	}
	else if(attempts < 381.0) { // > 10 fps (yellow)
		heat_level = mix(510.0, 580.0, (attempts - 191.0) / 190.0);
	}
	else if(attempts < 761.0) { // > 5 fps (red)
		heat_level = mix(580.0, 645.0, (attempts - 381.0) / 380.0);
	}
	else if(attempts < 3801.0) { // > 1 fps (dark red)
		heat_level = mix(645.0, 781.0, (attempts - 761.0) / 3040.0);
	}
	else heat_level = 782.0; // > 0 fps (black)

	vec3 heat_color = waveLengthToRGB(heat_level);
	heatColor = vec4(heat_color, 1.0);
}