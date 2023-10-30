struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Camera {
    vec3 lookfrom;
    vec3 lookdir;

    float pitch;
    float yaw;

    float fov;
    float camera_speed;

    int max_depth;
};

struct Hit {
    float t;
    vec3  normal;
    vec3  color;
};

struct AABB {
    vec3 min;
    vec3 max;
};

struct BVH_Node {
    AABB aabb;
    int left_id;
    int right_id;
};

struct Sphere {
	vec3  center;
	float radius;
};

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 normal;
    vec3 color;
    AABB bounding_box;
};

layout(std430, binding = 0) buffer SSBO_Camera {
    Camera camera;
};

layout(std430, binding = 1) buffer SSBO_Triangles {
    Triangle triangles[];
};