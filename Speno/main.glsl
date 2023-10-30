#version 460

layout(location = 0) out vec4 diffuse_and_seed;
layout(location = 1) out vec4 normal_and_depth;

uniform sampler2D previous_diffuse_and_seed_texture;
uniform sampler2D previous_normal_and_depth_texture;
uniform vec2 u_resolution;
uniform uint u_frame;
uniform uint u_frame_seed;
uniform float u_time;
uniform float u_delta_time;

#define PI 3.14159265
#define TAU 6.28318531
#define EPSILON 0.002
#define MAXIMUM_DISTANCE 16777215.0

#include "structures.glsl"
#include "random.glsl"
#include "functions.glsl"
#include "intersections.glsl"
#include "pathtracing.glsl"

void main() {
    setRootRandomSeed();

    Ray ray = getRay();

    vec3 color = clamp(pathtrace(ray), 0.0, 255.0);
    if(any(isnan(color))) color = vec3(0);

    vec3 previous_color = texelFetch(previous_diffuse_and_seed_texture, ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 final_color = mix(previous_color, color, 1.0 / float(u_frame));

	diffuse_and_seed = vec4(final_color, 1.0);
	normal_and_depth = vec4(0.0, 1.0, 0.0, 1.0);
}