#define HitData() HitData(false, MAXIMUM_FLOAT, -1, vec3(-1.0), vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0), 0.0);
struct HitData {
    bool was_intersection;
    float t;

    int triangle_index;
    vec3 uvw;

    vec3 position;
    vec3 normal;

    vec3 color;
    vec3 emission;
    float roughness;
};