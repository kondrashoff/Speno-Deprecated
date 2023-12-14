void setupSTBN() {
    stbn.use  = u_samples <=   STBN_SIZE_Z;
    stbn.use2 = u_samples <= 2*STBN_SIZE_Z;
    stbn.c = ivec2(gl_FragCoord.xy) % STBN_SIZE;

    uint first_layer = u_frame % STBN_SIZE_Z;
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
    float v = texelFetch(stbn_scalar_texture, ivec3(stbn.c, stbn.cscalar), 0).r;
    stbn.cscalar = (stbn.cscalar + 1u) % STBN_SIZE_Z;
    return v;
}

float getVec1STBN() {
    float v = texelFetch(stbn_vec1_texture, ivec3(stbn.c, stbn.cvec1), 0).r;
    stbn.cvec1 = (stbn.cvec1 + 1u) % STBN_SIZE_Z;
    return v;
}

vec2 getVec2STBN() {
    vec2 v = texelFetch(stbn_vec2_texture, ivec3(stbn.c, stbn.cvec2), 0).rg;
    stbn.cvec2 = (stbn.cvec2 + 1u) % STBN_SIZE_Z;
    return v;
}

vec3 getVec3STBN() {
    vec3 v = texelFetch(stbn_vec3_texture, ivec3(stbn.c, stbn.cvec3), 0).rgb;
    stbn.cvec3 = (stbn.cvec3 + 1u) % STBN_SIZE_Z;
    return v;
}

float getUnitvec1STBN() {
    float v = texelFetch(stbn_unitvec1_texture, ivec3(stbn.c, stbn.cunitvec1), 0).r;
    stbn.cunitvec1 = (stbn.cunitvec1 + 1u) % STBN_SIZE_Z;
    return v;
}

vec2 getUnitvec2STBN() {
    vec2 v = texelFetch(stbn_unitvec2_texture, ivec3(stbn.c, stbn.cunitvec2), 0).rg;
    stbn.cunitvec2 = (stbn.cunitvec2 + 1u) % STBN_SIZE_Z;
    return v;
}

vec3 getUnitvec3STBN() {
    vec3 v = texelFetch(stbn_unitvec3_texture, ivec3(stbn.c, stbn.cunitvec3), 0).rgb;
    stbn.cunitvec3 = (stbn.cunitvec3 + 1u) % STBN_SIZE_Z;
    return v;
}

vec4 getUnitvec3cosineSTBN() {
    vec4 v = texelFetch(stbn_unitvec3_cosine_texture, ivec3(stbn.c, stbn.cunitvec3_cosine), 0);
    stbn.cunitvec3_cosine = (stbn.cunitvec3_cosine + 1u) % STBN_SIZE_Z;
    return v;
}

vec4 getUnitvec3hdriSTBN() {
    vec4 v = texelFetch(stbn_unitvec3_hdri_texture, ivec3(stbn.c, stbn.cunitvec3_hdri), 0);
    stbn.cunitvec3_cosine = (stbn.cunitvec3_hdri + 1u) % STBN_SIZE_Z;
    return v;
}