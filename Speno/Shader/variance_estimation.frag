#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 texCoords;
uniform vec2 resolution;

layout(location = 0) out float gbufferVariance;

layout(bindless_sampler) uniform sampler2D accumulatedTexture;
layout(bindless_sampler) uniform sampler2D momentTexture;
layout(bindless_sampler) uniform sampler2D normalTexture;
layout(bindless_sampler) uniform sampler2D positionTexture;

#include "defines.glsl"
#include "denoising_util.glsl"
#include "color_functions.glsl"

void main() {
    float depth = texture(positionTexture, texCoords).w;
	if(depth < 0.0) return;

	vec3 normal = texture(normalTexture, texCoords).rgb;
	vec2 moment = texture(momentTexture, texCoords).rg;

	vec2 depth_gradient = vec2(dFdx(depth), dFdy(depth));
	float lum = luminance(safeLoad(accumulatedTexture, texCoords).rgb);
	float new_value = lum * lum;
	gbufferVariance = moment.x - new_value;

	const float spatial_variance_cutoff = 4.0;
	if(moment.g > spatial_variance_cutoff) return;

	float weight_sum = 0.0;
    float moment_sum = 0.0;
	float lum_sum = 0.0;

	for(int i = 0; i < estimation_kernel.length(); i++) {
        vec2 offset = estimation_kernel[i].xy / resolution;
		vec2 offset_coord = texCoords + offset;

		if(offset_coord.x < 0.0 || offset_coord.y < 0.0 || offset_coord.x > 1.0 || offset_coord.y > 1.0) continue;

		float temp_depth = texture(positionTexture, offset_coord).w;
		if(temp_depth < 0.0) continue;

	    float temp_lum = luminance(safeLoad(accumulatedTexture, offset_coord).rgb);
		vec3 temp_normal = texture(normalTexture, offset_coord).rgb;
        
		float normal_weight = normalWeight(normal, temp_normal);
		float depth_weight = depthWeight(depth, temp_depth, depth_gradient, offset);
		float lum_weight = luminanceWeight(lum, temp_lum);

		float weight = max(normal_weight * depth_weight * lum_weight, EPSILON);
		//float weight = max(estimation_kernel[i].z * normal_weight * depth_weight * lum_weight, EPSILON);

		weight_sum += weight;
		moment_sum += moment.x * weight;
		lum_sum += temp_lum * weight;
    }

	moment_sum /= weight_sum;
	lum_sum /= weight_sum;

	const float aggressiveness = 4.0;
	float aggro_factor = aggressiveness / min(moment.g, aggressiveness);

	gbufferVariance = aggro_factor * max(moment_sum - pow(lum_sum, 2.0), EPSILON);
}

/*void main() {
    float depth = texture(depthTexture, texCoords).r;
	if(depth < 0.0) return;
	
	vec3 normal = texture(normalTexture, texCoords).rgb;
	vec2 moment = texture(momentTexture, texCoords).rg;

	vec2 depth_gradient = vec2(dFdx(depth), dFdy(depth));
	float lum = luminance(safeLoad(accumulatedTexture, texCoords).rgb);
	float new_value = lum * lum;
	gbufferVariance = moment.x - new_value;

	float weight_sum = 0.0;
    float moment_sum = 0.0;
	float lum_sum = 0.0;

	for(int i = 0; i < estimation_kernel.length(); i++) {
        vec2 offset = estimation_kernel[i].xy / resolution;
		vec2 offset_coord = texCoords + offset;

		if(offset_coord.x < 0.0 || offset_coord.y < 0.0 || offset_coord.x > 1.0 || offset_coord.y > 1.0) continue;

		float temp_depth = texture(depthTexture, offset_coord).r;
		if(temp_depth < 0.0) continue;

	    float temp_lum = luminance(safeLoad(accumulatedTexture, offset_coord).rgb);
		float temp_moment = texture(momentTexture, offset_coord).x;
		vec3 temp_normal = texture(normalTexture, offset_coord).rgb;
        
		float normal_weight = normalWeight(normal, temp_normal);
		float depth_weight = depthWeight(depth, temp_depth, depth_gradient, offset);
		float lum_weight = luminanceWeight(lum, temp_lum);
		float weight = max(normal_weight * depth_weight * lum_weight, EPSILON);

		weight_sum += weight;
		moment_sum += temp_moment * weight;
		lum_sum += temp_lum * weight;
    }

	moment_sum /= weight_sum;
	lum_sum /= weight_sum;

	const float aggressiveness = 1.0;
	float aggro_factor = aggressiveness / min(moment.g, aggressiveness);

	gbufferVariance = aggro_factor * max(moment_sum - pow(lum_sum, 2.0), EPSILON);
}*/

/*void main() {
    float depth = texture(positionTexture, texCoords).w;
	if(depth < 0.0) return;
	
	vec3 normal = texture(normalTexture, texCoords).rgb;
	vec2 moment = texture(momentTexture, texCoords).rg;

	float lum = normalizedluminance(safeLoad(accumulatedTexture, texCoords).rgb);
	float new_value = lum * lum;
	gbufferVariance = moment.x - new_value;

	const float spatial_variance_cutoff = 4.0;
	if(moment.g > spatial_variance_cutoff) return;

	const float normal_phi = 1e-2;
	const float depth_phi = max(5e-3, 1e-8) * 3.0;
	const float color_phi = 1.0e1;

	float weight_sum = 0.0;
    float moment_sum = 0.0;
	float lum_sum = 0.0;

	for(int i = 0; i < estimation_kernel.length(); i++) {
        vec2 offset = estimation_kernel[i].xy / resolution;
		vec2 offset_coord = texCoords + offset;

		if(offset_coord.x < 0.0 || offset_coord.y < 0.0 || offset_coord.x > 1.0 || offset_coord.y > 1.0) continue;

		float temp_depth = texture(positionTexture, offset_coord).w;
		if(temp_depth < 0.0) continue;

	    float temp_lum = normalizedluminance(safeLoad(accumulatedTexture, offset_coord).rgb);
		vec3 temp_normal = texture(normalTexture, offset_coord).rgb;
        
		float normal_weight = pow(max(dot(normal, temp_normal), 0.0), normal_phi);
		float depth_weight = offset == vec2(0.0) ? 0.0 : abs(depth - temp_depth) / (depth_phi * length(offset * resolution));
		float luminance_weight = abs(lum - temp_lum) / color_phi;
		float weight = exp(-luminance_weight - depth_weight - normal_weight);
        
		weight_sum += weight;
        moment_sum += moment.x * weight;
        lum_sum += temp_lum * weight;
    }

	moment_sum /= weight_sum;
	lum_sum /= weight_sum;

	const float aggressiveness = 4.0;
	float aggro_factor = aggressiveness / min(moment.g, aggressiveness);

	gbufferVariance = aggro_factor * max(moment_sum - pow(lum_sum, 2.0), EPSILON);
}*/