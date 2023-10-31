#version 460

out vec4 FragColor;

uniform sampler2D diffuse_texture;
uniform sampler2D normal_texture;

vec3 clamp_color(in vec3 color) {
	float maximum = max(color.r, max(color.g, color.b));

	if(maximum <= 1.0) {
		return color;
	}

	return color / maximum;
}

float luminance(in vec3 color) {
	return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color) {
	float white = 2.0;
	float luma = luminance(color);
	float toneMappedLuma = luma * (1.0 + luma / (white*white)) / (1.0 + luma);
	color *= toneMappedLuma / luma;
	return color;
}

// Source:
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3 ACESInputMat = mat3(
    vec3(0.59719, 0.35458, 0.04823),
    vec3(0.07600, 0.90834, 0.01566),
    vec3(0.02840, 0.13383, 0.83777)
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat = mat3(
    vec3(1.60475, -0.53108, -0.07367),
    vec3(-0.10208, 1.10813, -0.00605),
    vec3(-0.00327, -0.07276, 1.07602)
);

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color) {
    color = color * ACESInputMat;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = color * ACESOutputMat;

    return color;
}

vec3 sRGB(vec3 color) {
    bvec3 cutoff = lessThan(color, vec3(0.0031308));
    vec3 higher = vec3(1.055) * pow(color, vec3(1.0 / 2.4)) - vec3(0.055);
    vec3 lower = color * vec3(12.92);

    return mix(higher, lower, cutoff);
}

void main() {
	vec3 color = texelFetch(diffuse_texture, ivec2(gl_FragCoord.xy), 0).rgb;

	color = whitePreservingLumaBasedReinhardToneMapping(color);
	color = clamp_color(color);
	color = ACESFitted(color);
	color = sRGB(color);
	
	FragColor = vec4(color, 1.0);
}