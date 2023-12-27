void setupSTBN() {
    stbn.c = ivec2(gl_FragCoord.xy) % stbn_resolution.xy;
    uvec2 in_grid = uvec2(gl_FragCoord.xy) / stbn_resolution.xy;
    uint grid_number = in_grid.x + in_grid.y * stbn_resolution.x;
    uint first_layer = (frame + grid_number) % stbn_resolution.z;
    stbn.cscalar = first_layer;
    stbn.cvec1 = first_layer;
    stbn.cvec2 = first_layer;
    stbn.cvec3 = first_layer;
    stbn.cunitvec1 = first_layer;
    stbn.cunitvec2 = first_layer;
    stbn.cunitvec3 = first_layer;
    stbn.cunitvec3_cosine = first_layer;
    stbn.cunitvec3_hdri = first_layer;
}

float getScalarSTBN() {
    float v = texelFetch(stbn_scalar, ivec3(stbn.c, stbn.cscalar), 0).r;
    stbn.cscalar = (stbn.cscalar + 1u) % stbn_resolution.z;
    return v;
}

float getVec1STBN() {
    float v = texelFetch(stbn_vec1, ivec3(stbn.c, stbn.cvec1), 0).r;
    stbn.cvec1 = (stbn.cvec1 + 1u) % stbn_resolution.z;
    return v;
}

vec2 getVec2STBN() {
    vec2 v = texelFetch(stbn_vec2, ivec3(stbn.c, stbn.cvec2), 0).rg;
    stbn.cvec2 = (stbn.cvec2 + 1u) % stbn_resolution.z;
    return v;
}

vec3 getVec3STBN() {
    vec3 v = texelFetch(stbn_vec3, ivec3(stbn.c, stbn.cvec3), 0).rgb;
    stbn.cvec3 = (stbn.cvec3 + 1u) % stbn_resolution.z;
    return v;
}

float getUnitvec1STBN() {
    float v = texelFetch(stbn_unitvec1, ivec3(stbn.c, stbn.cunitvec1), 0).r;
    stbn.cunitvec1 = (stbn.cunitvec1 + 1u) % stbn_resolution.z;
    return v;
}

vec2 getUnitvec2STBN() {
    vec2 v = texelFetch(stbn_unitvec2, ivec3(stbn.c, stbn.cunitvec2), 0).rg;
    stbn.cunitvec2 = (stbn.cunitvec2 + 1u) % stbn_resolution.z;
    return v;
}

vec3 getUnitvec3STBN() {
    vec3 v = texelFetch(stbn_unitvec3, ivec3(stbn.c, stbn.cunitvec3), 0).rgb;
    stbn.cunitvec3 = (stbn.cunitvec3 + 1u) % stbn_resolution.z;
    return v;
}

vec4 getUnitvec3cosineSTBN() {
    vec4 v = texelFetch(stbn_unitvec3_cosine, ivec3(stbn.c, stbn.cunitvec3_cosine), 0);
    stbn.cunitvec3_cosine = (stbn.cunitvec3_cosine + 1u) % stbn_resolution.z;
    return v;
}

vec4 getUnitvec3hdriSTBN() {
    vec4 v = texelFetch(stbn_unitvec3_hdri, ivec3(stbn.c, stbn.cunitvec3_hdri), 0);
    stbn.cunitvec3_cosine = (stbn.cunitvec3_hdri + 1u) % stbn_resolution.z;
    return v;
}