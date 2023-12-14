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

bool intersectTriagnle(inout Hit hit, in Ray ray, in Triangle triangle) {
    if(dot(ray.direction, triangle.normal) > 0.0) return false;

    vec3 e1 = triangle.v1 - triangle.v0;
    vec3 e2 = triangle.v2 - triangle.v0;
    vec3 pvec = cross(ray.direction, e2);
    float det = dot(e1, pvec);

    if(det <= EPSILON) return false;

    vec3 tvec = ray.origin - triangle.v0;

    float u = dot(tvec, pvec);
    if(u < 0.0 || u > det) return false;

    vec3 qvec = cross(tvec, e1);
    float v = dot(ray.direction, qvec);
    if(v < 0.0 || u + v > det) return false;

    float t = dot(e2, qvec) / det;

    if(t < 0.0 || t > hit.t) return false;

    hit.t = t;
    hit.normal = triangle.normal;

    return true;
}

bool intersectTwoSidedTriangle(inout Hit hit, in Ray ray, in Triangle triangle) {
    vec3 e1 = triangle.v1 - triangle.v0;
    vec3 e2 = triangle.v2 - triangle.v0;
    vec3 pvec = cross(ray.direction, e2);
    float det = dot(e1, pvec);

    if(abs(det) < EPSILON) return false;

    float inv_det = 1.0 / det;
    vec3 tvec = ray.origin - triangle.v0;

    float u = inv_det * dot(tvec, pvec);
    if(u < 0.0 || u > 1.0) return false;

    vec3 qvec = cross(tvec, e1);
    float v = inv_det * dot(ray.direction, qvec);

    if(v < 0.0 || u + v > 1.0) return false;

    float t = dot(e2, qvec) * inv_det;

    if(t < 0.0 || t > hit.t) return false;

    hit.t = t;
    hit.normal = faceforward(triangle.normal, ray.direction, triangle.normal);

    return true;
}

bool intersectAABB(inout Hit hit, in Ray ray, in AABB aabb) {
    vec3 inv_dir = 1.0 / ray.direction;
    vec3 tbot = inv_dir * (aabb.min - ray.origin);
    vec3 ttop = inv_dir * (aabb.max - ray.origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    float t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    float t1 = min(t.x, t.y);

    if(t1 < max(t0, 0.0)) return false;

    vec3 center = 0.5 * (aabb.min + aabb.max);
    vec3 center_to_point = ray.origin + t0 * ray.direction - center;
    vec3 half_size = 0.5 * (aabb.max - aabb.min);

    hit.t = t0;
    hit.normal = normalize(sign(center_to_point) * step(-EPSILON, abs(center_to_point) - half_size));

    return true;
}

bool testAABB(in Ray ray, in AABB aabb, inout Interval ray_t) {
    vec3 inv_dir = 1.0 / ray.direction;
    vec3 tbot = inv_dir * (aabb.min - ray.origin);
    vec3 ttop = inv_dir * (aabb.max - ray.origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    float t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    float t1 = min(t.x, t.y);

    //if(t1 < max(t0, 0.0) || ray_t.max < t0 || ray_t.min > t1 || ray_t.max < t1 || ray_t.min > t0 || (ray_t.max < t1 && ray_t.min < t0) || (ray_t.max < t1 && ray_t.min > t0)) return false;
    if(t1 < max(t0, 0.0) || ray_t.max < t1) return false;

    ray_t.min = t0;
    ray_t.max = t1;

    return true;
}

bool intersectSceneBVH(inout Hit hit, in Ray ray) {
    bool is_hit = false;
    hit.t = MAXIMUM_DISTANCE;
    
    int stack[MAX_BVH_STACK_SIZE];
    int stack_size = 0;
    int current_node_id = nodes.length() - 1;

    Interval ray_t = Interval(-MAXIMUM_DISTANCE, MAXIMUM_DISTANCE);
    BVH_Node current_node = nodes[current_node_id];

    while (true) {
        current_node = nodes[current_node_id];

        int left_index = current_node.left_id;
        int right_index = current_node.right_id;

        if(left_index < 0) {
            int triangle_id = -left_index - 1;
            Triangle triangle = triangles[triangle_id];

            if (intersectTwoSidedTriangle(hit, ray, triangle)) {
                ray_t.max = hit.t;
                hit.color = vec3(0.3);
                is_hit = true;
                return true;
            }
            else {
                if (stack_size <= 0) return is_hit;
                current_node_id = stack[--stack_size];
            }
        }
        else {
            BVH_Node left = nodes[left_index];
            BVH_Node right = nodes[right_index];

            bool hit_left = testAABB(ray, left.aabb, ray_t);
            bool hit_right = testAABB(ray, right.aabb, ray_t);

            if(hit_left) {
                if(hit_right) {
                    current_node_id = right_index;
                    if(stack_size < MAX_BVH_STACK_SIZE) stack[stack_size++] = left_index;
                    else {
                        int hash_id = left_index;
                        int hash_id2;
                        for(int x = MAX_BVH_STACK_SIZE - 1; x >= 0; x--) {
                            hash_id2 = stack[x];
                            stack[x] = hash_id;
                            hash_id = hash_id2;
                        }
                    }
                }
                else {
                    current_node_id = left_index;
                }
            }
            else if(hit_right) {
                current_node_id = right_index;
            }
            else {
                if (stack_size <= 0) return is_hit;
                current_node_id = stack[--stack_size];
            }
        }
    }

    return is_hit;
}

bool continue_condition(in Ray ray, in vec3 position, in float max_pos) {
    return (position.x >= 0.0     || ray.origin.x < 0.0    ) &&
           (position.x <= max_pos || ray.origin.x > max_pos) &&
           (position.y >= 0.0     || ray.origin.y < 0.0    ) &&
           (position.y <= 255.0   || ray.origin.y > 255.0  ) &&
           (position.z >= 0.0     || ray.origin.z < 0.0    ) &&
           (position.z <= max_pos || ray.origin.z > max_pos);
}

bool intersectScene3DDA(inout Hit hit, in Ray ray) {
    hit.t = MAXIMUM_DISTANCE;

    vec3 position = floor(ray.origin);
    vec3 rayInverse = 1.0 / ray.direction;
    vec3 raySign = sign(ray.direction);
    vec3 dist = (position - ray.origin + 0.5 + raySign * 0.5) * rayInverse;
    vec3 minMask = vec3(0.0);

    int chunks_total = chunks.length();
    int chunks_size = int(sqrt(chunks_total));
    float max_pos = float(chunks_size * 16);

    for (uint i = 0u; i < 128u && continue_condition(ray, position, max_pos); i++) {
        int block = 0;

        if(position.x >= 0.0 && position.z >= 0.0 && position.y > 0.0 && position.x < max_pos && position.z < max_pos && position.y < 255.0) {
            ivec3 ipos = ivec3(position);
            int chunk_number = (ipos.x / 16) * chunks_size + (ipos.z / 16);
            if(chunk_number >= 0 && chunk_number < chunks_total) {
                ivec3 array_pos = ipos % ivec3(16, 255, 16);
                block = chunks[chunk_number].block[array_pos.x][array_pos.y][array_pos.z];
            }
        }
        
        if (block != 0) {
            vec3 minPos = (position - ray.origin + 0.5 - 0.5 * vec3(raySign)) * rayInverse;
            float intersection = max(minPos.x, max(minPos.y, minPos.z));
            
            vec3 normal = -minMask * raySign;

            hit.t = intersection;
            hit.normal = faceforward(normal, ray.direction, normal);

            vec3 intr_pos = -(ray.origin + hit.t * ray.direction);
            vec3 abs_normal = abs(hit.normal);
            vec3 noise_pos = intr_pos;

            if(block == 1) {
                if(abs_normal.x > abs_normal.y && abs_normal.x > abs_normal.z) {
                    hit.color = texture(voxel_textures, vec3(intr_pos.zy, 0)).rgb;
                }
                else if(abs_normal.y > abs_normal.z) {
                    hit.color = texture(voxel_textures, vec3(intr_pos.xz, 0)).rgb;
                }
                else {
                    hit.color = texture(voxel_textures, vec3(intr_pos.xy, 0)).rgb;
                }
            }
            else if(block == 2) {
                if(abs_normal.x > abs_normal.y && abs_normal.x > abs_normal.z) {
                    hit.color = texture(voxel_textures, vec3(intr_pos.zy, 2)).rgb;
                }
                else if(abs_normal.y > abs_normal.z) {
                    if(normal.y < 0.0) hit.color = texture(voxel_textures, vec3(intr_pos.xz, 3)).rgb;
                    else hit.color = texture(voxel_textures, vec3(intr_pos.xz, 1)).rgb;
                }
                else {
                    hit.color = texture(voxel_textures, vec3(intr_pos.xy, 2)).rgb;
                }
            }
            else if(block == 3) {
                if(abs_normal.x > abs_normal.y && abs_normal.x > abs_normal.z) {
                    hit.color = texture(voxel_textures, vec3(intr_pos.zy, 3)).rgb;
                }
                else if(abs_normal.y > abs_normal.z) {
                    hit.color = texture(voxel_textures, vec3(intr_pos.xz, 3)).rgb;
                }
                else {
                    hit.color = texture(voxel_textures, vec3(intr_pos.xy, 3)).rgb;
                }
            }
            else {
                int index = block;

                if(abs_normal.x > abs_normal.y && abs_normal.x > abs_normal.z) {
                    hit.color = texture(voxel_textures, vec3(intr_pos.zy, index)).rgb;
                }
                else if(abs_normal.y > abs_normal.z) {
                    hit.color = texture(voxel_textures, vec3(intr_pos.xz, index)).rgb;
                }
                else {
                    hit.color = texture(voxel_textures, vec3(intr_pos.xy, index)).rgb;
                }
            }

            return true;
        }

        minMask = step(dist.xyz, dist.yzx) * step(dist.xyz, dist.zxy);
        dist += minMask * raySign * rayInverse;
        position += minMask * raySign;
    }

    return false;
}

bool intersectSceneDefault(inout Hit hit, in Ray ray) {
    bool is_hit = false;
    hit.t = MAXIMUM_DISTANCE;

    uint triangles_number = triangles.length();
    for(uint i = 0u; i < triangles_number; i++) {
        Triangle triangle = triangles[i];

        if(intersectTwoSidedTriangle(hit, ray, triangle)) {
            hit.color = triangle.color;
            hit.color = vec3(0.36, 0.05, 0.085);
            is_hit = true;
        }
    }

    if(is_hit) {
        vec3 noise_pos = 0.5 * (ray.origin + ray.direction * hit.t);
        float n1 = fbm(2.0 * noise_pos, 8);
        n1 = 0.33 + 0.33 * (1.0 + fbm(2.0 * n1 + noise_pos + vec3(847, 84, 489), 8));
        hit.color *= n1;
        hit.color = mix(hit.color, vec3(0.12, 0.45, 0.15), pow(dot(hit.normal, vec3(0, 1, 0)), 2.0));
    }

    return is_hit;
}

bool intersectScene(inout Hit hit, in Ray ray, in bool use_voxels) {
    is_volume = false;
    is_light = false;

    if(use_voxels) return intersectScene3DDA(hit, ray);
    else return intersectSceneDefault(hit, ray);
}