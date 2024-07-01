layout(bindless_sampler) uniform sampler2DArray stbn_scalar;
layout(bindless_sampler) uniform sampler2DArray stbn_vec1;
layout(bindless_sampler) uniform sampler2DArray stbn_vec2;
layout(bindless_sampler) uniform sampler2DArray stbn_vec3;
layout(bindless_sampler) uniform sampler2DArray stbn_unitvec1;
layout(bindless_sampler) uniform sampler2DArray stbn_unitvec2;
layout(bindless_sampler) uniform sampler2DArray stbn_unitvec3;
layout(bindless_sampler) uniform sampler2DArray stbn_unitvec3_cosine;

float getRandomScalar(int layer) {
	float v = texelFetch(stbn_scalar, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0).r;
	    
	return v;
}
	
float getRandomVec1(int layer) {
	float v = texelFetch(stbn_vec1, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0).r;
	    
	return v;
}
	
vec2 getRandomVec2(int layer) {
	vec2 v = texelFetch(stbn_vec2, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0).rg;
	    
	return v;
}
	
vec3 getRandomVec3(int layer) {
	vec3 v = texelFetch(stbn_vec3, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0).rgb;
	   
	return v;
}
	
float getRandomUnitvec1(int layer) {
	float v = texelFetch(stbn_unitvec1, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0).r;
	    
	return v;
}
	
vec2 getRandomUnitvec2(int layer) {
	vec2 v = texelFetch(stbn_unitvec2, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0).rg;
	    
	v = 2.0 * v - 1.0;
	
	return v;
}
	
vec3 getRandomUnitvec3(int layer) {
	vec3 v = texelFetch(stbn_unitvec3, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0).rgb;
	    
	v = 2.0 * v - 1.0;
	
	return v;
}
	
vec4 getRandomUnitvec3cosine(int layer) {
	vec4 v = texelFetch(stbn_unitvec3_cosine, ivec3(gl_FragCoord.xy, layer + frame) % 128, 0);
	    
	v.xyz = 2.0 * v.xyz - 1.0;
	
	return v;
}

//struct STBNSTATE {
//    ivec2 coord;
//    int layer;
//    int offset_scalar;
//    int offset_vec1;
//    int offset_vec2;
//    int offset_vec3;
//    int offset_unitvec1;
//    int offset_unitvec2;
//    int offset_unitvec3;
//    int offset_unitvec3_cosine;
//} stbn_state;
//
//void setupSTBN() {
//    stbn_state.coord = ivec2(gl_FragCoord.xy) % 128;
//
//    int first_layer = frame % 128;
//    stbn_state.layer = first_layer;
//    stbn_state.offset_scalar = first_layer;
//    stbn_state.offset_vec1 = first_layer;
//    stbn_state.offset_vec2 = first_layer;
//    stbn_state.offset_vec3 = first_layer;
//    stbn_state.offset_unitvec1 = first_layer;
//    stbn_state.offset_unitvec2 = first_layer;
//    stbn_state.offset_unitvec3 = first_layer;
//    stbn_state.offset_unitvec3_cosine = first_layer;
//}
//
//float getRandomScalar() {
//    int layer = stbn_state.layer + stbn_state.offset_scalar++;
//    float v = texelFetch(stbn_scalar, ivec3(stbn_state.coord, layer), 0).r;
//    
//    return v;
//}
//
//float getRandomVec1() {
//    int layer = stbn_state.layer + stbn_state.offset_vec1++;
//    float v = texelFetch(stbn_vec1, ivec3(stbn_state.coord, layer), 0).r;
//    
//    return v;
//}
//
//vec2 getRandomVec2() {
//    int layer = stbn_state.layer + stbn_state.offset_vec2++;
//    vec2 v = texelFetch(stbn_vec2, ivec3(stbn_state.coord, layer), 0).rg;
//    
//    return v;
//}
//
//vec3 getRandomVec3() {
//    int layer = stbn_state.layer + stbn_state.offset_vec3++;
//    vec3 v = texelFetch(stbn_vec3, ivec3(stbn_state.coord, layer), 0).rgb;
//   
//    return v;
//}
//
//float getRandomUnitvec1() {
//    int layer = stbn_state.layer + stbn_state.offset_unitvec1++;
//    float v = texelFetch(stbn_unitvec1, ivec3(stbn_state.coord, layer), 0).r;
//    
//    return v;
//}
//
//vec2 getRandomUnitvec2() {
//    int layer = stbn_state.layer + stbn_state.offset_unitvec2++;
//    vec2 v = texelFetch(stbn_unitvec2, ivec3(stbn_state.coord, layer), 0).rg;
//    
//    v = 2.0 * v - 1.0;
//
//    return v;
//}
//
//vec3 getRandomUnitvec3() {
//    int layer = stbn_state.layer + stbn_state.offset_unitvec3++;
//    vec3 v = texelFetch(stbn_unitvec3, ivec3(stbn_state.coord, layer), 0).rgb;
//    
//    v = 2.0 * v - 1.0;
//
//    return v;
//}
//
//vec4 getRandomUnitvec3cosine() {
//    int layer = stbn_state.layer + stbn_state.offset_unitvec3_cosine++;
//    vec4 v = texelFetch(stbn_unitvec3_cosine, ivec3(stbn_state.coord, layer), 0);
//    
//    v.xyz = 2.0 * v.xyz - 1.0;
//
//    return v;
//}