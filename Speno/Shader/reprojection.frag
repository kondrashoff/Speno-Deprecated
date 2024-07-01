#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 texCoords;
uniform vec2 resolution;

layout(location = 0) out vec3 gbufferAccumulated;
layout(location = 1) out vec4 gbufferGodrays;
layout(location = 2) out vec2 gbufferMoment;

layout(bindless_sampler) uniform sampler2D diffuseTexture;
layout(bindless_sampler) uniform sampler2D godraysTexture;
layout(bindless_sampler) uniform sampler2D positionTexture;

layout(bindless_sampler) uniform sampler2D previousGodraysTexture;
layout(bindless_sampler) uniform sampler2D previousDenoisedTexture;
layout(bindless_sampler) uniform sampler2D previousMomentTexture;
layout(bindless_sampler) uniform sampler2D previousPositionTexture;

#include "defines.glsl"
#include "shared_engine_data.glsl"
#include "camera.glsl"
#include "color_functions.glsl"
#include "denoising_util.glsl"

float evalAdaptive(sampler2D current, sampler2D previous, vec2 prev_uv) {
	const int kernel_size = 7;
	const float sigma = 3.0;

	//const float cstr = 4.0;
	//const float deps = 0.1;
    //const float dstr = 1.0;

	float sigmasq = 2.0 * sigma * sigma;
	float invsigma = 1.0 / (PI * sigmasq);

	float weight_sum = 0.0;
	float lum_max = 0.0;

	//float cprevlum = luminance(safeLoad(previous, prev_uv).rgb);
	//float cprevdepth = texture(previousPositionTexture, prev_uv).w;

	//float clum = luminance(safeLoad(current, texCoords).rgb);
	//float cdepth = texture(positionTexture, texCoords).w;

	//vec2 dprevgrad = abs(vec2(dFdx(cprevdepth), dFdy(cprevdepth)));
	//vec2 dgrad = abs(vec2(dFdx(cdepth), dFdy(cdepth)));

	float lum_prev = 0.0;
	for(int i = -kernel_size; i <= kernel_size; i++) {
		for(int j = -kernel_size; j <= kernel_size; j++) {
			vec2 offset = vec2(i, j);
			vec2 coord = prev_uv + offset / resolution;

			if(coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0) continue;

			float tlum = normalizedluminance(safeLoad(previous, coord).rgb);
			//float tdepth = texture(previousPositionTexture, coord).w;

			//float lum_weight = exp(-abs(cprevlum - tlum) / cstr);
			//float depth_weight = exp(-abs(cprevdepth - tdepth) / (dstr * dot(dprevgrad, abs(offset)) + deps));
			float kernel_weight = invsigma * exp(-dot(offset, offset) / sigmasq);
			float weight = kernel_weight;// * lum_weight * depth_weight;

			lum_max = max(lum_max, tlum);
			lum_prev += weight * tlum;
			weight_sum += weight;
		}
	}

	lum_prev /= weight_sum;
	weight_sum = 0.0;

    float lum_curr = 0.0;
	for(int i = -kernel_size; i <= kernel_size; i++) {
		for(int j = -kernel_size; j <= kernel_size; j++) {
			vec2 offset = vec2(i, j);
			vec2 coord = texCoords + offset / resolution;

			if(coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0) continue;

			float tlum = normalizedluminance(safeLoad(current, coord).rgb);
			//float tdepth = texture(positionTexture, coord).w;

			//float lum_weight = exp(-abs(clum - tlum) / cstr);
			//float depth_weight = exp(-abs(cdepth - tdepth) / (dstr * dot(dgrad, abs(offset)) + deps));
			float kernel_weight = invsigma * exp(-dot(offset, offset) / sigmasq);
			float weight = kernel_weight;// * lum_weight * depth_weight;

			lum_max = max(lum_max, tlum);
			lum_curr += weight * tlum;
			weight_sum += weight;
		}
	}

	lum_curr /= weight_sum;

	float lum_diff = abs(lum_curr - lum_prev) / lum_max;
	//float lum_diff = abs(lum_curr - lum_prev) / max(lum_curr, lum_prev);
	return lum_diff * lum_diff;
}

void main() {
	Ray godrays_ray = getRay(texCoords);
	vec4 current_godrays = safeLoad(godraysTexture, texCoords);
	gbufferGodrays = vec4(current_godrays.rgb, 1.0);

	vec2 prev_godrays_uv = getPreviousUV(godrays_ray.origin + godrays_ray.direction * abs(current_godrays.w));

	if(all(greaterThan(prev_godrays_uv, vec2(0))) && all(lessThan(prev_godrays_uv, vec2(1)))) {
		vec4 previous_godrays = texture(previousGodraysTexture, prev_godrays_uv);
		float adaptive_factor = 1.0;//evalAdaptive(godraysTexture, previousGodraysTexture, prev_godrays_uv);
		float max_accum = clamp(1.0 / adaptive_factor, 1.0, current_godrays.w < 0.0 ? 3.0 : 4096.0);

		gbufferGodrays.w = min(previous_godrays.w + 1.0, max_accum);
		gbufferGodrays.rgb = mix(previous_godrays.rgb, gbufferGodrays.rgb, 1.0 / gbufferGodrays.w);
	}

	vec3 current_diffuse = safeLoad(diffuseTexture, texCoords).rgb;
    float new_value = pow(luminance(current_diffuse), 2.0);

	vec4 pdata = texture(positionTexture, texCoords);
	vec3 position = pdata.xyz;
	float depth = pdata.w;

	// Standart output
	gbufferAccumulated = current_diffuse;
	gbufferMoment = vec2(new_value, 1.0);

	if(depth < 0.0) return; // Cutoff

	// Previous reprojected uv
	vec2 prev_uv = getPreviousUV(position);

	if(any(lessThan(prev_uv, vec2(0))) || any(greaterThan(prev_uv, vec2(1)))) return; // Cutoff

	// Previous frame depth
	float previous_depth = texture(previousPositionTexture, prev_uv).w;

	if(previous_depth < 0.0) return; // Cutoff

	Ray ray = getPreviousRay(prev_uv);
	vec3 reprojected_position = ray.origin + ray.direction * previous_depth;
	float offset = length(reprojected_position - position);

	if(offset > depth * REPROJECTION_DELTA) return; // Cutoff

	vec3 previous_denoised = texture(previousDenoisedTexture, prev_uv).rgb;
	vec2 previous_moment = texture(previousMomentTexture, prev_uv).xy;
	
	float adaptive_factor = evalAdaptive(diffuseTexture, previousDenoisedTexture, prev_uv);
	float depth_factor = min((previous_depth * previous_depth) / (depth * depth), 1.0);
	float max_accum = clamp(1.0 / (adaptive_factor * depth_factor), 1.0, 4096.0);

	gbufferMoment.g = min(previous_moment.g + 1.0, max_accum);
	gbufferMoment.r = mix(previous_moment.r, new_value, 1.0 / gbufferMoment.g);
	gbufferAccumulated = mix(previous_denoised, current_diffuse, 1.0 / gbufferMoment.g);
}