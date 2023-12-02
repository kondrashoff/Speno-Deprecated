#version 460
precision highp float;

layout(location = 0) out vec4 gbuffer_diffuse;
layout(location = 1) out vec4 gbuffer_albedo;
layout(location = 2) out vec4 gbuffer_normal;
layout(location = 3) out vec4 gbuffer_position;
layout(location = 4) out vec4 gbuffer_velocity;
layout(location = 5) out vec4 gbuffer_light;

uniform sampler2D previous_diffuse_texture;
uniform sampler2D previous_albedo_texture;
uniform sampler2D previous_normal_texture;
uniform sampler2D previous_position_texture;
uniform sampler2D previous_light_texture;

uniform sampler2DArray stbn_scalar_texture;
uniform sampler2DArray stbn_vec1_texture;
uniform sampler2DArray stbn_vec2_texture;
uniform sampler2DArray stbn_vec3_texture;
uniform sampler2DArray stbn_unitvec1_texture;
uniform sampler2DArray stbn_unitvec2_texture;
uniform sampler2DArray stbn_unitvec3_texture;
uniform sampler2DArray stbn_unitvec3_cosine_texture;

uniform sampler2D hdri_texture;
uniform sampler2DArray voxel_game_textures;

uniform vec2 u_resolution;
uniform uint u_frame;
uniform uint u_samples;
uniform uint u_frame_seed;
uniform float u_time;
uniform float u_delta_time;

#define PI                 3.14159265
#define TAU                6.28318531
#define EPSILON            0.002
#define REPROJ_DELTA       0.05
#define MAXIMUM_DISTANCE   16777215.0
#define MAX_BVH_STACK_SIZE 23

#define SKY_TYPE_BLACK     0
#define SKY_TYPE_DEFAULT   1
#define SKY_TYPE_REALISTIC 2

highp uint random_seed;

uint stbn_scalar_shift;
uint stbn_vec1_shift;
uint stbn_vec2_shift;
uint stbn_vec3_shift;
uint stbn_unitvec1_shift;
uint stbn_unitvec2_shift;
uint stbn_unitvec3_shift;
uint stbn_unitvec3_cosine_shift;

bool is_volume = false; // TODO: убрать эту хрень и наконец-то заменить на нормальные материалы
bool is_light = false;

#include "structures.glsl"
#include "random.glsl"
#include "noise.glsl"
#include "functions.glsl"
#include "intersections.glsl"
#include "pathtracing.glsl"

void main() {
    //float c = texture(stbn_scalar_texture, vec3(gl_FragCoord.xy / 128.0, u_frame % 64u)).r;
    //float c = texelFetch(stbn_vec1_texture, ivec3(gl_FragCoord.xy, u_frame % 64u) % 128, 0).r;
    //vec2 c = texelFetch(stbn_vec2_texture, ivec3(gl_FragCoord.xy, u_frame % 64u) % 128, 0).rg;
    //vec3 c = texelFetch(stbn_vec3_texture, ivec3(gl_FragCoord.xy, u_frame % 64u) % 128, 0).rgb;
    //float c = texelFetch(stbn_unitvec1_texture, ivec3(gl_FragCoord.xy, u_frame % 64u) % 128, 0).r;
    //vec2 c = texelFetch(stbn_unitvec2_texture, ivec3(gl_FragCoord.xy, u_frame % 64u) % 128, 0).rg;
    //vec3 c = texelFetch(stbn_unitvec3_texture, ivec3(gl_FragCoord.xy, u_frame % 64u) % 128, 0).rgb;
    //vec3 c = texelFetch(stbn_unitvec3_cosine_texture, ivec3(gl_FragCoord.xy, u_frame % 64u) % 128, 0).rgb;

    //bool cond1 = any(lessThanEqual(c, vec3(0.0)));
    //bool cond2 = any(greaterThanEqual(c, vec3(1.0)));

    //if(cond1 || cond2) c = vec3(1, 0, 0);
    //else c = vec3(0);

    //gbuffer_diffuse = vec4(c);
    //return;

    setRandomSeed();

    vec3 color = vec3(0);

    Ray ray;
    for(uint s = 0u; s < camera.samples_per_pixel; s++) {
        ray = getRay();
        color += pathtrace(ray);
    }
    color /= float(camera.samples_per_pixel);

    if(u_samples < 2u) {
        vec2 reproj_uv = getUV(old_camera, gbuffer_position.rgb);
        reproj_uv.x /= u_resolution.x / u_resolution.y;
        reproj_uv = (1.0 + reproj_uv) * 0.5;

        if(reproj_uv.x >= 0.0 && reproj_uv.x <= 1.0 && reproj_uv.y >= 0.0 && reproj_uv.y <+ 1.0) {
            vec4 reproj_position = texture(previous_position_texture, reproj_uv, 0);

            if(reproj_position.a >= 0.0) {
                Ray oldray = getRay(old_camera, reproj_uv);
          	    vec3 old_pos = oldray.origin + oldray.direction * reproj_position.a;
                
                float ofs = length(old_pos - gbuffer_position.rgb);
                if(ofs < REPROJ_DELTA) {
                    gbuffer_velocity = vec4(reproj_uv, 1.0, 1.0);
                }
            }
        }
    }
        
    float samples = 1.0 / max(1.0, float(u_samples - 1u));
        
    vec3 previous_color    = texelFetch(previous_diffuse_texture,  ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 previous_albedo   = texelFetch(previous_albedo_texture,   ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 previous_normal   = texelFetch(previous_normal_texture,   ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 previous_position = texelFetch(previous_position_texture, ivec2(gl_FragCoord.xy), 0).rgb;
    
    gbuffer_diffuse.rgb   = mix(previous_color,    color,                samples);
    gbuffer_albedo.rgb    = mix(previous_albedo,   gbuffer_albedo.rgb,   samples);
    gbuffer_normal.rgb    = mix(previous_normal,   gbuffer_normal.rgb,   samples);
    gbuffer_position.rgb  = mix(previous_position, gbuffer_position.rgb, samples);
}