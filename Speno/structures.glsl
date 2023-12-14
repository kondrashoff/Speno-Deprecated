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

    uint max_depth;
    uint samples_per_pixel;
};

struct Sky {
	int type;
	vec3 sun_direction;
	int sun_quality_i;
	int sun_quality_j;
    float pitch;
    float yaw;
};

struct Hit {
    float t;
    vec3 normal;
    vec3 color;
    float emmitance;
};

struct Interval {
    float min;
    float max;
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

struct Chunk {
    int block[16][255][16];
};

struct STBN {
    uint cscalar;
    uint cvec1, cvec2, cvec3;
    uint cunitvec1, cunitvec2, cunitvec3;
    uint cunitvec3_cosine, cunitvec3_hdri;
    ivec2 c;
    bool use;
    bool use2;
} stbn;

layout(std430, binding = 0) readonly buffer SSBO_Camera {
    Camera camera;
};

layout(std430, binding = 1) readonly buffer SSBO_Triangles {
    Triangle triangles[];
};

layout(std430, binding = 2) readonly buffer SSBO_BVH_Nodes {
    BVH_Node nodes[];
};

layout(std430, binding = 3) readonly buffer SSBO_Sky {
    Sky sky;
};

layout(std430, binding = 4) readonly buffer SSBO_Old_Camera {
    Camera old_camera;
};

layout(std430, binding = 5) readonly buffer SSBO_Blocks {
    Chunk chunks[];
};