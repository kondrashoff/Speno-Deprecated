bool intersectTriangle(inout HitData hit, Ray ray, Triangle triangle) {
    vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e2 = triangle.vertices[2] - triangle.vertices[0];
    vec3 pvec = cross(ray.direction, e2);
    float det = dot(e1, pvec);

    float inv_det = 1.0 / det;
    vec3 tvec = ray.origin - triangle.vertices[0];

    float u = inv_det * dot(tvec, pvec);
    if(u < 0.0 || u > 1.0) return false;

    vec3 qvec = cross(tvec, e1);
    float v = inv_det * dot(ray.direction, qvec);
    if(v < 0.0) return false;

	float UpV = u + v;
	if(UpV > 1.0) return false;

    float t = dot(e2, qvec) * inv_det;
    if(t < 0.0 || t > hit.t) return false;

    float w = 1.0 - UpV;

	Material mat = materials[triangle.material_id];
	if(mat.use_diffuse_tex) {
		vec2 uv0 = mat.diffuse_scale.x * (mat.diffuse_offset.x + u) * triangle.uvs[1];
		vec2 uv1 = mat.diffuse_scale.y * (mat.diffuse_offset.y + v) * triangle.uvs[2];
		vec2 uv2 = mat.diffuse_scale.z * (mat.diffuse_offset.z + w) * triangle.uvs[0];
		vec2 tex_coords = uv0 + uv1 + uv2;
	
		vec4 tex_diffuse = texture(sampler2D(mat.diffuse_tex_handle), tex_coords);
		if(tex_diffuse.a < 1.0) return false; // TODO: change this (for transparent materials)

		hit.color = tex_diffuse.rgb;
	}
	else hit.color = mat.diffuse_color;

	if(mat.use_emissive_tex) {
		vec2 uv0 = mat.emissive_scale.x * (mat.emissive_offset.x + u) * triangle.uvs[1];
		vec2 uv1 = mat.emissive_scale.y * (mat.emissive_offset.y + v) * triangle.uvs[2];
        vec2 uv2 = mat.emissive_scale.z * (mat.emissive_offset.z + w) * triangle.uvs[0];
		vec2 tex_coords = uv0 + uv1 + uv2;

		vec3 tex_emissive = texture(sampler2D(mat.emissive_tex_handle), tex_coords).rgb;
        if(luminance(tex_emissive) > 0.0) {
			hit.emission = tex_emissive;

			if(hit.color != tex_emissive && hit.color != vec3(0.0)) {
				hit.emission *= hit.color;
			}

			hit.color = vec3(1.0);
		}
	}
	else if(luminance(mat.emission_color) > 0.0) {
		hit.emission = mat.emission_color;
		
		if(hit.color != hit.emission && hit.color != vec3(0.0)) {
			hit.emission *= hit.color;
		}

		hit.color = vec3(1.0);
	}
	else hit.emission = vec3(0.0);

	vec3 normal = u * triangle.normals[1] + v * triangle.normals[2] + w * triangle.normals[0];
	if(mat.use_normal_tex) {
        vec2 uv0 = mat.normal_scale.x * (mat.normal_offset.x + u) * triangle.uvs[1];
		vec2 uv1 = mat.normal_scale.y * (mat.normal_offset.y + v) * triangle.uvs[2];
		vec2 uv2 = mat.normal_scale.z * (mat.normal_offset.z + w) * triangle.uvs[0];
        vec2 tex_coords = uv0 + uv1 + uv2;

		vec3 tex_normal = 2.0 * texture(sampler2D(mat.normal_tex_handle), tex_coords).rgb - 1.0;
		tex_normal.y = abs(tex_normal.y);
		
		hit.normal = onbBuildFromW(normal) * normalize(tex_normal);
	}
	else hit.normal = normal;

	if(mat.use_roughness_tex) {
        vec2 uv0 = mat.roughness_scale.x * (mat.roughness_offset.x + u) * triangle.uvs[1];
		vec2 uv1 = mat.roughness_scale.y * (mat.roughness_offset.y + v) * triangle.uvs[2];
		vec2 uv2 = mat.roughness_scale.z * (mat.roughness_offset.z + w) * triangle.uvs[0];
		vec2 tex_coords = uv0 + uv1 + uv2;
	
		hit.roughness = texture(sampler2D(mat.roughness_tex_handle), tex_coords).r;
	}
	else hit.roughness = mat.roughness;

    hit.t = t;
	hit.uvw = vec3(u, v, w);
	hit.position = ray.origin + hit.t * ray.direction;
    hit.was_intersection = true;

	hit.normal = faceforward(hit.normal, ray.direction, hit.normal);
	hit.color = sRGBtoLinear(hit.color);
	hit.emission = sRGBtoLinear(hit.emission);
	hit.emission *= 25.0;
	//hit.emission = vec3(0.0);

    return true;
}

bool intersectTriangle(inout HitData hit, Ray ray, int triangle_index) {
	Triangle triangle = triangles[triangle_index];
    vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e2 = triangle.vertices[2] - triangle.vertices[0];
    vec3 pvec = cross(ray.direction, e2);
    float det = dot(e1, pvec);

    float inv_det = 1.0 / det;
    vec3 tvec = ray.origin - triangle.vertices[0];

    float u = inv_det * dot(tvec, pvec);
    if(u < 0.0 || u > 1.0) return false;

    vec3 qvec = cross(tvec, e1);
    float v = inv_det * dot(ray.direction, qvec);
    if(v < 0.0) return false;

	float UpV = u + v;
	if(UpV > 1.0) return false;

    float t = dot(e2, qvec) * inv_det;
    if(t < 0.0 || t > hit.t) return false;

    float w = 1.0 - UpV;

	// TODO: remove this when transparent materials come out
	Material mat = materials[triangle.material_id];
	if(mat.use_diffuse_tex) {
		vec2 uv0 = mat.diffuse_scale.x * (mat.diffuse_offset.x + u) * triangle.uvs[1];
		vec2 uv1 = mat.diffuse_scale.y * (mat.diffuse_offset.y + v) * triangle.uvs[2];
		vec2 uv2 = mat.diffuse_scale.z * (mat.diffuse_offset.z + w) * triangle.uvs[0];
		vec2 tex_coords = uv0 + uv1 + uv2;
	
		vec4 tex_diffuse = texture(sampler2D(mat.diffuse_tex_handle), tex_coords);
		if(tex_diffuse.a < 1.0) return false; // TODO: change this (for transparent materials)

		hit.color = tex_diffuse.rgb;
	}
	else hit.color = mat.diffuse_color;

    hit.t = t;
	hit.triangle_index = triangle_index;
	hit.uvw = vec3(u, v, w);
    hit.was_intersection = true;

    return true;
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

bool traverseBLAS(int root_index, inout HitData hit, Ray ray, bool only_first_hit) {
	BVHNode node = bvh_nodes[root_index];
    int stack[BLAS_STACK_SIZE];
    uint ptr = 0u;

	while(true) {
		ptr = min(BLAS_STACK_SIZE - 1, ptr);
		int index = floatBitsToInt(node.data1.w);

		if (index > -1) {
			BVHNode left = bvh_nodes[index];
			BVHNode right = bvh_nodes[index + 1];

			vec2 leftHit = testAABB(hit, ray, left.data1.xyz, left.data2.xyz);
			vec2 rightHit = testAABB(hit, ray, right.data1.xyz, right.data2.xyz);

			if (leftHit.y > 0.0 && rightHit.y > 0.0) {
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
			else if (leftHit.y > 0.0) {
				node = left;
				continue;
			}
			else if (rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else if(intersectTriangle(hit, ray, -index - 1) && only_first_hit) return true;

		if (ptr != 0u) node = bvh_nodes[stack[--ptr]];
		else return hit.was_intersection;
	}
}

bool traverse(inout HitData hit, Ray ray, float t, bool only_first_hit) {
	hit = HitData();
	hit.t = t;
	ray.inverse_direction = 1.0 / ray.direction;

	int num_nodes = bvh_nodes.length();
	if (num_nodes == 0) return false;

	BVHNode node = bvh_nodes[num_nodes - 1];
    int stack[TLAS_STACK_SIZE];
    uint ptr = 0;

	while(true) {
		ptr = min(TLAS_STACK_SIZE - 1, ptr);
		int index = floatBitsToInt(node.data1.w);

		if (index > -1) {
			BVHNode left = bvh_nodes[index];
			BVHNode right = bvh_nodes[index + 1];

			vec2 leftHit = testAABB(hit, ray, left.data1.xyz, left.data2.xyz);
			vec2 rightHit = testAABB(hit, ray, right.data1.xyz, right.data2.xyz);

			if (leftHit.y > 0.0 && rightHit.y > 0.0) {
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
			else if (leftHit.y > 0.0) {
				node = left;
				continue;
			}
			else if (rightHit.y > 0.0) {
				node = right;
				continue;
			}
		}
		else if(traverseBLAS(meshes[-index - 1].last_node_index, hit, ray, only_first_hit) && only_first_hit) return true;

		if (ptr != 0u) node = bvh_nodes[stack[--ptr]];
		else break;
	}

	if(!hit.was_intersection) return false;

	Triangle intersected_triangle = triangles[hit.triangle_index];
	Material mat = materials[intersected_triangle.material_id];

	vec3 normal = hit.uvw.x * intersected_triangle.normals[1] + hit.uvw.y * intersected_triangle.normals[2] + hit.uvw.z * intersected_triangle.normals[0];
	hit.position = ray.origin + hit.t * ray.direction;

	/*if (mat.use_diffuse_tex) {
		vec2 uv0 = mat.diffuse_scale.x * (mat.diffuse_offset.x + hit.uvw.x) * intersected_triangle.uvs[1];
		vec2 uv1 = mat.diffuse_scale.y * (mat.diffuse_offset.y + hit.uvw.y) * intersected_triangle.uvs[2];
		vec2 uv2 = mat.diffuse_scale.z * (mat.diffuse_offset.z + hit.uvw.z) * intersected_triangle.uvs[0];
		vec2 tex_coords = uv0 + uv1 + uv2;

		hit.color = texture(sampler2D(mat.diffuse_tex_handle), tex_coords).rgb;
	}
	else hit.color = mat.diffuse_color;*/

	if(mat.use_emissive_tex) {
		vec2 uv0 = mat.emissive_scale.x * (mat.emissive_offset.x + hit.uvw.x) * intersected_triangle.uvs[1];
		vec2 uv1 = mat.emissive_scale.y * (mat.emissive_offset.y + hit.uvw.y) * intersected_triangle.uvs[2];
        vec2 uv2 = mat.emissive_scale.z * (mat.emissive_offset.z + hit.uvw.z) * intersected_triangle.uvs[0];
		vec2 tex_coords = uv0 + uv1 + uv2;

		vec3 tex_emissive = texture(sampler2D(mat.emissive_tex_handle), tex_coords).rgb;
        if(luminance(tex_emissive) > 0.0) {
			hit.emission = tex_emissive;

			if(hit.color != tex_emissive && hit.color != vec3(0.0)) {
				hit.emission *= hit.color;
			}

			hit.color = vec3(1.0);
		}
	}
	else if(luminance(mat.emission_color) > 0.0) {
		hit.emission = mat.emission_color;
		
		if(hit.color != hit.emission && hit.color != vec3(0.0)) {
			hit.emission *= hit.color;
		}

		hit.color = vec3(1.0);
	}
	else hit.emission = vec3(0.0);

	if(mat.use_normal_tex) {
        vec2 uv0 = mat.normal_scale.x * (mat.normal_offset.x + hit.uvw.x) * intersected_triangle.uvs[1];
		vec2 uv1 = mat.normal_scale.y * (mat.normal_offset.y + hit.uvw.y) * intersected_triangle.uvs[2];
		vec2 uv2 = mat.normal_scale.z * (mat.normal_offset.z + hit.uvw.z) * intersected_triangle.uvs[0];
        vec2 tex_coords = uv0 + uv1 + uv2;

		vec3 tex_normal = 2.0 * texture(sampler2D(mat.normal_tex_handle), tex_coords).rgb - 1.0;
		tex_normal.y = abs(tex_normal.y);

		hit.normal = onbBuildFromW(normal) * normalize(tex_normal);
	}
	else hit.normal = normal;

	if(mat.use_roughness_tex) {
        vec2 uv0 = mat.roughness_scale.x * (mat.roughness_offset.x + hit.uvw.x) * intersected_triangle.uvs[1];
		vec2 uv1 = mat.roughness_scale.y * (mat.roughness_offset.y + hit.uvw.y) * intersected_triangle.uvs[2];
		vec2 uv2 = mat.roughness_scale.z * (mat.roughness_offset.z + hit.uvw.z) * intersected_triangle.uvs[0];
		vec2 tex_coords = uv0 + uv1 + uv2;
	
		hit.roughness = texture(sampler2D(mat.roughness_tex_handle), tex_coords).r;
	}
	else hit.roughness = mat.roughness;

	hit.normal = faceforward(hit.normal, ray.direction, hit.normal);
	hit.color = sRGBtoLinear(hit.color);
	hit.emission = sRGBtoLinear(hit.emission);
	hit.emission *= 25.0;
	//hit.emission = vec3(0.0);

	return true;
}