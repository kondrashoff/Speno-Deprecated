#version 460

layout(location = 0) out vec4 gbuffer_diffuse;
layout(location = 1) out vec4 gbuffer_normal;

uniform sampler2D previous_diffuse_texture;
uniform sampler2D previous_normal_texture;
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

    vec3 previous_color = texelFetch(previous_diffuse_texture, ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 previous_normal = texelFetch(previous_normal_texture, ivec2(gl_FragCoord.xy), 0).rgb;

	gbuffer_diffuse.rgb = mix(previous_color, color, 1.0 / float(max(u_frame - 1u, 1u)));
    gbuffer_normal.rgb = mix(previous_normal, gbuffer_normal.rgb, 1.0 / float(max(u_frame - 1u, 1u)));
}