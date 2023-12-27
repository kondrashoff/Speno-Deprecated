bool intersectSphere(inout Hit hit, in Ray ray, in Sphere sphere) {
    vec3 k  = ray.origin - sphere.center;
    float b = dot(k, ray.direction);
    float c = dot(k, k) - sphere.radius * sphere.radius;
    float d = b * b - c;

    if (d < 0.0) return false;
    
    float sqrtfd = sqrt(d);
    float t1 = -b + sqrtfd;
    float t2 = -b - sqrtfd;

    float min_t = min(t1, t2);
    float max_t = max(t1, t2);

    float t = (min_t >= 0.0) ? min_t : max_t;

    if(t < 0.0 || t > hit.t) return false;

    hit.t = t;
    hit.normal = ((ray.origin + ray.direction * t) - sphere.center) / sphere.radius;
    
    return true;
}

bool intersectGroundPlane(inout Hit hit, in Ray ray, in float height) {
    if(dot(ray.direction, vec3(0, 1, 0)) > 0.0) return false;

    float t = (height - ray.origin.y) / ray.direction.y;

    if(t < 0.0 || t > hit.t) return false;

    hit.t = t;
    hit.normal = vec3(0, 1, 0);

    return true;
}

bool intersectTwoSidedTriangle(inout Hit hit, in Ray ray, in Triangle triangle) {
    vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e2 = triangle.vertices[2] - triangle.vertices[0];
    vec3 pvec = cross(ray.direction, e2);
    float det = dot(e1, pvec);

    if(abs(det) < EPSILON) return false;

    float inv_det = 1.0 / det;
    vec3 tvec = ray.origin - triangle.vertices[0];

    float u = inv_det * dot(tvec, pvec);
    if(u < 0.0 || u > 1.0) return false;

    vec3 qvec = cross(tvec, e1);
    float v = inv_det * dot(ray.direction, qvec);

    if(v < 0.0 || u + v > 1.0) return false;

    float t = dot(e2, qvec) * inv_det;

    if(t < 0.0 || t > hit.t) return false;

    hit.t = t;
    hit.normal = faceforward(triangle.normal, ray.direction, triangle.normal);
    hit.uv = vec2(u, v);

    return true;
}

bool intersectTriangle(inout Hit hit, in Ray ray, in Triangle triangle) {
    if(use_two_sided_geometry) return intersectTwoSidedTriangle(hit, ray, triangle);

    if(dot(ray.direction, triangle.normal) > 0.0) return false;

    vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e2 = triangle.vertices[2] - triangle.vertices[0];
    vec3 pvec = cross(ray.direction, e2);
    float det = dot(e1, pvec);

    if(det <= EPSILON) return false;

    vec3 tvec = ray.origin - triangle.vertices[0];

    float u = dot(tvec, pvec);
    if(u < 0.0 || u > det) return false;

    vec3 qvec = cross(tvec, e1);
    float v = dot(ray.direction, qvec);
    if(v < 0.0 || u + v > det) return false;

    float t = dot(e2, qvec) / det;

    if(t < 0.0 || t > hit.t) return false;

    hit.t = t;
    hit.normal = triangle.normal;
    hit.uv = vec2(u, v);

    return true;
}

bool intersectAABB(inout Hit hit, in Ray ray, in AABB aabb) {
    vec3 inv_dir = 1.0 / ray.direction;
    vec3 tbot = inv_dir * (aabb.minimum - ray.origin);
    vec3 ttop = inv_dir * (aabb.maximum - ray.origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    float t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    float t1 = min(t.x, t.y);

    if(t1 < max(t0, 0.0)) return false;

    vec3 center = 0.5 * (aabb.minimum + aabb.maximum);
    vec3 center_to_point = ray.origin + t0 * ray.direction - center;
    vec3 half_size = 0.5 * (aabb.maximum - aabb.minimum);

    hit.t = t0;
    hit.normal = normalize(sign(center_to_point) * step(-EPSILON, abs(center_to_point) - half_size));

    return true;
}

float testAABB(in Ray ray, in AABB aabb) {
    vec3 invdir = 1.0 / ray.direction;

	vec3 n = (aabb.minimum - ray.origin) * invdir;
	vec3 f = (aabb.maximum - ray.origin) * invdir;

	vec3 tmin = min(f, n);
	vec3 tmax = max(f, n);

	float t0 = max(tmin.x, max(tmin.y, tmin.z));
	float t1 = min(tmax.x, min(tmax.y, tmax.z));

	return (t1 >= t0) ? (t0 > 0.f ? t0 : t1) : -1.0;
}

bool intersectSceneBVH(inout Hit hit, in Ray ray) {
    bool is_hit = false;
    hit.t = MAXIMUM_FLOAT;

    int stack[BVH_STACK_SIZE];
    int ptr = 0;
    stack[ptr++] = -1;

    int idx = bvh_nodes.length() - 1;
    float leftHit = 0.0;
    float rightHit = 0.0;

	while(idx > -1) {
		int n = idx;
		BVH_Node node = bvh_nodes[n];

		int leftIndex = node.left_id;
		int rightIndex = node.right_id;
		bool isLeaf = leftIndex < 0 || rightIndex < 0;

		if(isLeaf) {
            int start_triangle = -leftIndex - 1;
            int end_triangle = -rightIndex - 1;
            for(int i = start_triangle; i <= end_triangle; i++) {
				Triangle triangle = triangles[i];
				if(intersectTriangle(hit, ray, triangle)) {
					hit.material.type = MATERIAL_LAMBERTIAN;
                    if(mesh_color_method == METHOD_TEXTURE_COLOR) hit.color = texture(mesh_texture, getTextureUV(hit, triangle)).rgb;
                    else if(mesh_color_method == METHOD_ATTRIBUTE_COLOR) hit.color = triangle.color;
                    else if(mesh_color_method == METHOD_SINGLE_COLOR) hit.color = mesh_color;
					is_hit = true;
				}
			}
		}
		else {
            AABB leftAABB = bvh_nodes[leftIndex].aabb;
            AABB rightAABB = bvh_nodes[rightIndex].aabb;

            leftHit = testAABB(ray, leftAABB);
            rightHit = testAABB(ray, rightAABB);

			if (leftHit > 0.0 && rightHit > 0.0) {
				int deferred = -1;
				if (leftHit > rightHit) {
					idx = rightIndex;
					deferred = leftIndex;
				}
				else {
					idx = leftIndex;
					deferred = rightIndex;
				}

				stack[ptr++] = deferred;
				continue;
			}
			else if (leftHit > 0) {
				idx = leftIndex;
				continue;
			}
			else if (rightHit > 0) {
				idx = rightIndex;
				continue;
			}
		}
		idx = stack[--ptr];
	}

    if(mesh_color_method == METHOD_AUTO_COLOR && is_hit) {
        vec3 noise_pos = 0.5 * (ray.origin + ray.direction * hit.t);
        float n1 = fbm(2.0 * noise_pos, 8);
        n1 = 0.33 + 0.33 * (1.0 + fbm(2.0 * n1 + noise_pos + vec3(847, 84, 489), 8));
        vec3 c1 = vec3(0.9, 0.2, 0.05) * n1;
        hit.color = mix(c1, vec3(0.9, 0.6, 0.05), pow(dot(hit.normal, vec3(0, 1, 0)), 2.0));
    }

	return is_hit;
}

bool intersectSceneDefault(inout Hit hit, in Ray ray) {
    bool is_hit = false;
    hit.t = MAXIMUM_FLOAT;

    uint triangles_number = triangles.length();
    for(uint i = 0u; i < triangles_number; i++) {
        Triangle triangle = triangles[i];

        if(intersectTriangle(hit, ray, triangle)) {
            hit.color = triangle.color;
            is_hit = true;
        }
    }

    return is_hit;
}

bool intersectScene(inout Hit hit, in Ray ray) {
	if(bvh_nodes.length() > 0) {
        return intersectSceneBVH(hit, ray);
    }
    else {
		return intersectSceneDefault(hit, ray);
	}
}