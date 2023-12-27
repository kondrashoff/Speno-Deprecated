#version 460 core
precision highp float;

layout(location = 0) out vec4 gbufferDiffuse;
layout(location = 1) out vec4 gbufferAlbedo;
layout(location = 2) out vec4 gbufferNormal;
layout(location = 3) out vec4 gbufferPosition;
in vec2 texCoords;

uniform sampler2D gbufferPreviousDiffuse;
uniform sampler2D gbufferPreviousPosition;

uniform ivec3 stbn_resolution;
uniform sampler2DArray stbn_scalar;
uniform sampler2DArray stbn_vec1;
uniform sampler2DArray stbn_unitvec1;
uniform sampler2DArray stbn_vec2;
uniform sampler2DArray stbn_unitvec2;
uniform sampler2DArray stbn_vec3;
uniform sampler2DArray stbn_unitvec3;
uniform sampler2DArray stbn_unitvec3_cosine;
uniform sampler2DArray stbn_unitvec3_hdri;

uniform vec2 resolution;

layout(std140) uniform SharedEngineData {
	float time;
	float delta_time;
	uint frame;
	
	int mesh_color_method;
	float hdri_luminance_sum;
	vec3 sun_direction;
    vec3 mesh_color;

    bool use_two_sided_geometry ;
	bool use_temporal_reprojection;
	bool use_pathtracing;
	bool use_shadows;
	bool use_pathtracing_ambient_occlusion;
	bool use_hdri;
	bool do_not_reproject_movement;
};

uniform sampler2D mesh_texture;
uniform sampler2D hdri_texture;
uniform sampler2D hdri_data_texture;

highp uint random_seed;
vec4 gbuffer_intersection;

#include "defines.glsl"
#include "structures.glsl"
#include "tonemap.glsl"
#include "stbn.glsl"
#include "random.glsl"
#include "noise.glsl"
#include "functions.glsl"
#include "intersections.glsl"
#include "pathtracing.glsl"

void main() {
	setupSTBN();
	//setRandomSeed();

	Ray ray = getRay();

	vec3 color = clamp(getRayColor(ray), 0.0, 128.0);

    bool reprojected = false;
    if(((use_pathtracing && use_temporal_reprojection) || (use_pathtracing_ambient_occlusion && use_temporal_reprojection)) && gbuffer_intersection.a > 0.0) {
        if(!do_not_reproject_movement || !any(greaterThan(abs(camera.lookfrom - camera.lookfrom_prev), vec3(EPSILON)))) {
            vec2 reproj = getUV();
            reproj.x /= resolution.x / resolution.y;
            reproj = (1.0 + reproj) * 0.5;
            if(reproj.x > 0.0 && reproj.x < 1.0 && reproj.y > 0.0 && reproj.y < 1.0) {
                vec4 reproj_position = texture(gbufferPreviousPosition, reproj);
                if(reproj_position.a > 0.0) {
                    Ray oldray = getPreviousRay(reproj);
          	        vec3 old_pos = oldray.origin + oldray.direction * reproj_position.a;
                
                    float ofs = length(old_pos - gbuffer_intersection.rgb);
                    if(ofs < REPROJ_DELTA) {
                        reprojected = true;
                        vec4 data = texture(gbufferPreviousDiffuse, reproj);
                        color = mix(data.rgb, color, 1.0 / data.a);
                        gbufferDiffuse = vec4(color, min(stbn_resolution.z, data.a + 1.0)); // A limitation, because in the future there will be more dynamics.
                    }
                }
            }
        }
    }

    if(!reprojected) {
        gbufferDiffuse = vec4(color, 1.0);
    }

    gbufferPosition = gbuffer_intersection;
}