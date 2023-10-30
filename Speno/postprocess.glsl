#version 460

out vec4 FragColor;

uniform sampler2D diffuse_and_seed_texture;
uniform sampler2D normal_and_depth_texture;

float luminance(in vec3 color) {
	return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

float luminance2(in vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 clamp_color(in vec3 color) {
	float maximum = max(color.r, max(color.g, color.b));

	if(maximum <= 1.0) {
		return color;
	}

	return color / maximum;
}

void main() {
	vec3 color = texelFetch(diffuse_and_seed_texture, ivec2(gl_FragCoord.xy), 0).rgb;

	FragColor = vec4(color, 1.0);
}