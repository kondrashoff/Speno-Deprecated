struct Triangle {
    mat3 vertices;
    mat3 normals;
    mat3x2 uvs;
    int material_id;
};

struct BVHNode {
    vec4 data1;
    vec4 data2;
};

struct Mesh {
    int last_node_index;
};

struct Material {
    vec3 diffuse_color;
    vec3 emission_color;
    float roughness;

    uvec2 diffuse_tex_handle;
    vec3 diffuse_offset;
    vec3 diffuse_scale;
    bool use_diffuse_tex;

    uvec2 emissive_tex_handle;
    vec3 emissive_offset;
    vec3 emissive_scale;
    bool use_emissive_tex;

    uvec2 normal_tex_handle;
    vec3 normal_offset;
    vec3 normal_scale;
    bool use_normal_tex;
    
    uvec2 roughness_tex_handle;
    vec3 roughness_offset;
    vec3 roughness_scale;
    bool use_roughness_tex;
};

layout(std430, binding = SSBO_TRIANGLES_BINDING) buffer TrianglesSSBO {
    Triangle triangles[];
};

layout(std430, binding = SSBO_LIGHTS_BINDING) buffer LightsSSBO {
    int light_ids[];
};

layout(std430, binding = SSBO_MATERIALS_BINDING) buffer MaterialsSSBO {
    Material materials[];
};

layout(std430, binding = SSBO_BVH_BINDING) buffer BVHSSBO {
    BVHNode bvh_nodes[];
};

layout(std430, binding = SSBO_MESHES_BINDING) buffer MeshesSSBO {
    Mesh meshes[];
};

struct AABB {
    vec3 minimum;
    vec3 maximum;
};

AABB getBVHRootBounds() {
    return AABB(vec3(0.0), vec3(0.0));
    //BVHNode node = bvh_nodes[bvh_nodes.length() - 1];
    //return AABB(node.data1.xyz, node.data2.xyz);
}