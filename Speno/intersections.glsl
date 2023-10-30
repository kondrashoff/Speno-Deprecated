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

float testAABB(in Ray ray, in AABB aabb) {
    vec3 rayInvDir = 1.0 / ray.direction;
    vec3 tbot = rayInvDir * (aabb.min - ray.origin);
    vec3 ttop = rayInvDir * (aabb.max - ray.origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    float t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    float t1 = min(t.x, t.y);

    return (t1 > max(t0, 0.0)) ? max(t0, EPSILON) : -1.0;
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

    is_hit = intersectSphere(hit, ray, Sphere(vec3(10, 0, 0), 1.0)) || is_hit;

    return is_hit;
}