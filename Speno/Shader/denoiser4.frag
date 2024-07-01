#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 texCoords;
uniform vec2 resolution;

layout(location = 0) out vec3 gbufferDenoised;
layout(location = 1) out float gbufferVariance;

layout(bindless_sampler) uniform sampler2D denoisedTexture;
layout(bindless_sampler) uniform sampler2D varianceTexture;
layout(bindless_sampler) uniform sampler2D positionTexture;
layout(bindless_sampler) uniform sampler2D normalTexture;

#include "denoising_util.glsl"
#include "color_functions.glsl"

vec3 denoise() {
    vec3 diffuse = safeLoad(denoisedTexture, texCoords).rgb;
	vec3 normal = texture(normalTexture, texCoords).rgb;
    float depth = texture(positionTexture, texCoords).w;

    if(depth < 0.0) return diffuse;
	vec2 depth_gradient = vec2(dFdx(depth), dFdy(depth));
    float lum = luminance(diffuse);

    float filtered_variance = 0.0;
    for(int i = 0; i < variance_kernel.length(); i++) {
        vec3 variance_filter = variance_kernel[i];
        vec2 offset = variance_filter.xy / resolution;
        vec2 coord = texCoords + offset;

        if(coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0) continue;

        float temp_variance = texture(varianceTexture, coord).r;

        float weight = variance_filter.z;
        filtered_variance += weight * temp_variance;
    }
    filtered_variance = sqrt(filtered_variance);
    if(isnan(filtered_variance) || filtered_variance < 1e-5) filtered_variance = 1e-5;

    vec3 color = vec3(0.0);
    float variance = 0.0;
	float weight_sum = 0.0;

	for(int i = 0; i < filter_kernel.length(); i++) {
		vec2 offset = filter_kernel[i].xy / resolution;
        vec2 coord = texCoords + 4.0 * offset;

        if(coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0) continue;

		float temp_depth = texture(positionTexture, coord).w;
		if(temp_depth < 0.0) continue;

	    vec3 temp_color = safeLoad(denoisedTexture, coord).rgb;
		float temp_variance = texture(varianceTexture, coord).r;
        vec3 temp_normal = texture(normalTexture, coord).rgb;
		float temp_lum = luminance(temp_color);

		float normal_weight = normalWeight(normal, temp_normal);
		float depth_weight = depthWeight(depth, temp_depth, depth_gradient, offset);
		float luminance_weight = luminanceWeight(lum, temp_lum, filtered_variance);
		float weight = normal_weight * depth_weight * luminance_weight;

        float kernel_weight = filter_kernel[i].z;
		color += kernel_weight * weight * temp_color;
        variance += pow(kernel_weight, 2.0) * pow(weight, 2.0) * temp_variance;
		weight_sum += weight * kernel_weight;
	}

    gbufferVariance = variance / pow(weight_sum, 2.0);
	return color / weight_sum;
}

void main() {
    gbufferDenoised = denoise();
}