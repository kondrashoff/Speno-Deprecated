#version 460
precision highp float;

#include "base.glsl"
#include "structures.glsl"
#include "stbn.glsl"
#include "random.glsl"
#include "noise.glsl"
#include "functions.glsl"
#include "intersections.glsl"
#include "pathtracing.glsl"

void main() {
#ifdef USE_GRID
    // decouple rendering, it can get a bit unresponsive
  	vec2 grid_size = vec2(96, 54); // increase these number if you have a good GPU
  	vec2 grid_count = floor(u_resolution / grid_size) + 1.0;
    vec2 in_grid = floor(gl_FragCoord.xy / grid_size);
    float grid_pos = in_grid.x + in_grid.y * grid_count.x;
    float grid_pos_max = grid_count.x*grid_count.y;
    if (mod(float(u_frame - 1u), grid_pos_max) != grid_pos) {
        gbuffer_diffuse  = texelFetch(previous_diffuse_texture,  ivec2(gl_FragCoord.xy), 0);
        gbuffer_albedo   = texelFetch(previous_albedo_texture,   ivec2(gl_FragCoord.xy), 0);
        gbuffer_normal   = texelFetch(previous_normal_texture,   ivec2(gl_FragCoord.xy), 0);
        gbuffer_position = texelFetch(previous_position_texture, ivec2(gl_FragCoord.xy), 0);
        return;
    }
#endif

    setupSTBN();
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

        if(reproj_uv.x >= 0.0 && reproj_uv.x <= 1.0 && reproj_uv.y >= 0.0 && reproj_uv.y <= 1.0) {
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
        
    
#ifdef USE_GRID    
    float acc_samples = texelFetch(previous_diffuse_texture, ivec2(gl_FragCoord.xy), 0).a + 1.0;
    float samples = 1.0 / acc_samples;
#else
    float samples = 1.0 / max(1.0, float(u_samples - 1u));
#endif

    vec3 previous_color    = texelFetch(previous_diffuse_texture,  ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 previous_albedo   = texelFetch(previous_albedo_texture,   ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 previous_normal   = texelFetch(previous_normal_texture,   ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 previous_position = texelFetch(previous_position_texture, ivec2(gl_FragCoord.xy), 0).rgb;
    
    gbuffer_diffuse.rgb   = mix(previous_color,    color,                samples);
    gbuffer_albedo.rgb    = mix(previous_albedo,   gbuffer_albedo.rgb,   samples);
    gbuffer_normal.rgb    = mix(previous_normal,   gbuffer_normal.rgb,   samples);
    gbuffer_position.rgb  = mix(previous_position, gbuffer_position.rgb, samples);

#ifdef USE_GRID
    gbuffer_diffuse.a = acc_samples;
#endif
}