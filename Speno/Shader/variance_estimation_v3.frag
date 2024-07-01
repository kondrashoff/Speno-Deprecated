#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 texCoords;
uniform vec2 resolution;

layout(location = 0) out vec3 gbufferVariance;

layout(bindless_sampler) uniform sampler2D diffuseTexture;
layout(bindless_sampler) uniform sampler2D previousVarianceTexture;
layout(bindless_sampler) uniform sampler2D reprojectedTexture;
layout(bindless_sampler) uniform sampler2D normalTexture;
layout(bindless_sampler) uniform sampler2D depthTexture;
layout(bindless_sampler) uniform sampler2D roughnessTexture;

#include "denoising_util.glsl"
#include "color_functions.glsl"

float estimateVariance(float mean2) {
    const float normal_phi = 1e-2;
	const float depth_phi = 1.5e-4;
	const float color_phi = 10.0;

	vec3 normal = texture(normalTexture, texCoords).rgb;
    float depth = texture(depthTexture, texCoords).r;
	float lum = luminance(safeLoad(diffuseTexture, texCoords).rgb);

    if(depth < 0.0) return vec2(0.0);

    float luma_sum = 0.0;
    float luma_sum2 = 0.0;
    float weight_sum = 0.0;

    for(int i = 0; i < estimation_kernel.length(); i++) {
        vec2 offset = estimation_kernel[i].xy / resolution;
		vec2 offset_coord = texCoords + offset;

		if(offset_coord.x < 0.0 || offset_coord.y < 0.0 || offset_coord.x > 1.0 || offset_coord.y > 1.0) continue;

		float temp_depth = texture(depthTexture, texCoords + offset).r;

		if(temp_depth < 0.0) continue;

	    float temp_luminance = luminance(safeLoad(diffuseTexture, texCoords + offset).rgb);
		vec3 temp_normal = texture(normalTexture, texCoords + offset).rgb;

		float normal_weight = pow(max(dot(normal, temp_normal), 0.0), normal_phi);
		float depth_weight = offset == vec2(0.0) ? 0.0 : abs(depth - temp_depth) / (depth_phi * length(offset * resolution));
		float luminance_weight = abs(lum - temp_luminance) / color_phi;
		float weight = abs(exp(-luminance_weight - depth_weight - normal_weight));
        
		float luma = weight * temp_luminance;
        luma_sum += luma;
        luma_sum2 += luma * luma;
		weight_sum += weight;
    }

    float final_luma = luma_sum / weight_sum;
    float final_luma2 = luma_sum / pow(weight_sum, 2.0);

    return vec2(final_luma, final_luma2);
}

void main() {
	float current_luminance = luminance(texture(diffuseTexture, texCoords).rgb);
	float new_value = current_luminance * current_luminance;

	vec2 reprojected_uv = texture(reprojectedTexture, texCoords).xy;
	if(reprojected_uv.x < 0.0) {
		
		gbufferVariance.r = 0.0;
		gbufferVariance.g = new_value;
		gbufferVariance.b = 1.0;
		return;
	}

	const float min_accumulated = 1.0;
	const float max_accumulated = 128.0;

	vec3 previousVariance = texture(previousVarianceTexture, reprojected_uv).rgb;
	float roughness = texture(roughnessTexture, texCoords).r;
	float maximum_accumulated_frames = mix(min_accumulated, max_accumulated, pow(roughness, 2.0));

    gbufferVariance.b = min(previousVariance.b + 1.0, maximum_accumulated_frames);
	gbufferVariance.g = mix(previousVariance.g, new_value, 1.0 / gbufferVariance.b);
    gbufferVariance.r = gbufferVariance.g - new_value;
}