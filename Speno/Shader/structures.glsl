struct STBN {
    uint cscalar;
    uint cvec1, cvec2, cvec3;
    uint cunitvec1, cunitvec2, cunitvec3;
    uint cunitvec3_cosine, cunitvec3_hdri;
    ivec2 c;
} stbn;

struct Camera {
    mat3 view;
    vec3 lookfrom;
    mat3 view_prev;
    vec3 lookfrom_prev;
    float tan_half_fov;
    float tan_half_fov_prev;
    uint max_depth;
};

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Material {
    uint type;
};

struct Hit {
    float t;
    vec3 normal;
    vec2 uv;
    vec3 color;
    Material material;
};

struct Sphere {
	vec3 center;
	float radius;
};

struct AABB {
    vec3 minimum;
    vec3 maximum;
};

struct Triangle {
    mat3 vertices;
    vec3 normal;
    mat3x2 uvs;
    vec3 color;
};

struct BVH_Node {
    AABB aabb;
    int left_id;
    int right_id;
};

layout(std430, binding = 0) buffer SSBO_Camera {
	Camera camera;
};

layout(std430, binding = 1) buffer SSBO_Triangles {
    Triangle triangles[];
};

layout(std430, binding = 2) buffer SSBO_BVH_Nodes {
	BVH_Node bvh_nodes[];
};