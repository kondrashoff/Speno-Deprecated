#version 460
precision highp float;

layout(location = 0) out vec4 gbuffer_diffuse;
layout(location = 1) out vec4 gbuffer_albedo;
layout(location = 2) out vec4 gbuffer_normal;
layout(location = 3) out vec4 gbuffer_position;

uniform sampler2D previous_diffuse_texture;
uniform sampler2D previous_albedo_texture;
uniform sampler2D previous_normal_texture;
uniform sampler2D previous_position_texture;

uniform sampler2D map_texture;
uniform sampler2D stone_texture;
uniform sampler2D grass_top_texture;
uniform sampler2D grass_side_texture;
uniform sampler2D dirt_texture;

uniform vec2 u_resolution;
uniform uint u_frame;
uniform uint u_frame_seed;
uniform float u_time;
uniform float u_delta_time;

#define SKY_TYPE_BLACK     0
#define SKY_TYPE_DEFAULT   1
#define SKY_TYPE_REALISTIC 2

#define PI                 3.14159265
#define TAU                6.28318531
#define EPSILON            0.002
#define REPROJ_DELTA       0.5
#define MAXIMUM_DISTANCE   16777215.0
#define MAX_BVH_STACK_SIZE 23

#include "structures.glsl"
#include "random.glsl"
#include "noise.glsl"
#include "functions.glsl"
#include "sky.glsl"
#include "intersections.glsl"
#include "pathtracing.glsl"

void main() {
    setRootRandomSeed();

    vec3 color = vec3(0);

    Ray ray;
    for(uint s = 0u; s < camera.samples_per_pixel; s++) {
        ray = getRay();
        color += pathtrace(ray);
    }
    color /= float(camera.samples_per_pixel);

    if(u_frame < 0u) {
        vec2 reproj_uv = getUV(old_camera, gbuffer_position.rgb);
        reproj_uv.x /= u_resolution.x / u_resolution.y;
        reproj_uv = (1.0 + reproj_uv) * 0.5;
       
        bool is_reprojected = false;
        if(reproj_uv.x >= 0.0 && reproj_uv.x <= 1.0 && reproj_uv.y >= 0.0 && reproj_uv.y <+ 1.0) {
            vec4 reproj_color    = texture(previous_diffuse_texture,  reproj_uv, 0);
            vec4 reproj_position = texture(previous_position_texture, reproj_uv, 0);

            if(reproj_position.a >= 0.0) {
                Ray oldray = getRay(old_camera, reproj_uv);
          	    vec3 old_pos = oldray.origin + oldray.direction * reproj_position.a;
                
                float ofs = length(old_pos - gbuffer_position.rgb);
                if(ofs < REPROJ_DELTA) {
                    if(gbuffer_diffuse.a < 1.0 / u_delta_time) {
                        gbuffer_diffuse.rgb = reproj_color.rgb + color;
                        gbuffer_diffuse.a   = reproj_color.a   + 1.0;
                    }
                    is_reprojected = true;
                }
            }
        }
    
        if(!is_reprojected) {
            gbuffer_diffuse = vec4(color, 1.0);
        }
    }
    else {
        float samples = 1.0 / max(1.0, float(u_frame - 1u));
        
        vec3 previous_color    = texelFetch(previous_diffuse_texture,  ivec2(gl_FragCoord.xy), 0).rgb;
        vec3 previous_albedo   = texelFetch(previous_albedo_texture,   ivec2(gl_FragCoord.xy), 0).rgb;
        vec3 previous_normal   = texelFetch(previous_normal_texture,   ivec2(gl_FragCoord.xy), 0).rgb;
        vec3 previous_position = texelFetch(previous_position_texture, ivec2(gl_FragCoord.xy), 0).rgb;

        gbuffer_diffuse.a     = 1.0;
	    gbuffer_diffuse.rgb   = mix(previous_color,    color,                samples);
        gbuffer_albedo.rgb    = mix(previous_albedo,   gbuffer_albedo.rgb,   samples);
        gbuffer_normal.rgb    = mix(previous_normal,   gbuffer_normal.rgb,   samples);
        gbuffer_position.rgb  = mix(previous_position, gbuffer_position.rgb, samples);
    }
}