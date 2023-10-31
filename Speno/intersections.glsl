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

    float invDet = 1.0 / det;
    vec3 tvec = ray.origin - triangle.v0;

    float u = invDet * dot(tvec, pvec);
    if(u < 0.0 || u > 1.0) return false;

    vec3 qvec = cross(tvec, e1);
    float v = invDet * dot(ray.direction, qvec);

    if(v < 0.0 || u + v > 1.0) return false;

    float t = dot(e2, qvec) * invDet;

    if(t < 0.0 || t > hit.t) return false;

    hit.t = t;
    hit.normal = faceforward(triangle.normal, ray.direction, triangle.normal);

    return true;
}

bool testAABB(in Ray ray, in AABB aabb, inout Interval ray_t) {
     for (int a = 0; a < 3; a++) {
        float invD = 1.0 / ray.direction[a];
        float orig = ray.origin[a];

        float t0 = (aabb.min[a] - orig) * invD;
        float t1 = (aabb.max[a] - orig) * invD;

        if (invD < 0.0) {
            float hash = t0;
            t0 = t1;
            t1 = hash;
        }
            
        if (t0 > ray_t.min) ray_t.min = t0;
        if (t1 < ray_t.max) ray_t.max = t1;

        if (ray_t.max <= ray_t.min)
            return false;
    }
    
    return true;
}

#define MAX_BVH_STACK_SIZE 64

bool intersectSceneBVH(inout Hit hit, in Ray ray) {
    bool is_hit = false;
    hit.t = MAXIMUM_DISTANCE;
    
    int stack[MAX_BVH_STACK_SIZE];
    int stack_size = 0;
    int current_node_id = nodes.length() - 1;
    int attempts = 64;

    Interval ray_t = Interval(-MAXIMUM_DISTANCE, MAXIMUM_DISTANCE);
    BVH_Node current_node = nodes[current_node_id];

    while (attempts > 0) {
        current_node = nodes[current_node_id];

        int left_index = current_node.left_id;
        int right_index = current_node.right_id;

        if(left_index < 0) {
            int triangle_id = -left_index - 1;

            Triangle triangle = triangles[triangle_id];

            if (intersectTwoSidedTriangle(hit, ray, triangle)) {
                attempts--;
                ray_t.max = hit.t;

                hit.color = triangle.color;
                is_hit = true;
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
            
            Interval hit_ray_t = ray_t;
            hit_ray_t.max = hit_left ? ray_t.min : ray_t.max;
            bool hit_right = testAABB(ray, right.aabb, hit_ray_t);
            if(hit_right) ray_t = hit_ray_t;

            if(hit_left) {
                if(hit_right) {
                    current_node_id = right_index;
                    if(stack_size < MAX_BVH_STACK_SIZE) stack[stack_size++] = left_index;
                }
                else {
                    current_node_id = left_index;
                    if(stack_size < MAX_BVH_STACK_SIZE) stack[stack_size++] = right_index;
                }
            }
            else {
                if (stack_size <= 0) return is_hit;
                current_node_id = stack[--stack_size];
            }
        }
    }

    return is_hit;
}

bool intersectScene(inout Hit hit, in Ray ray) {
    bool is_hit = false;
    hit.t = MAXIMUM_DISTANCE;

    uint triangles_number = triangles.length();
    
    for(uint i = 0u; i < triangles_number; i++) {
        Triangle triangle = triangles[i];

        if(intersectTwoSidedTriangle(hit, ray, triangle)) {
            hit.color = triangle.color;
            is_hit = true;
        }
    }

    return is_hit;
}