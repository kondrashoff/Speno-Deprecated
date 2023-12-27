#version 460 core

out vec4 fragColor;
in vec2 texCoords;

uniform sampler2D denoisedImage;
uniform sampler2D gbufferAlbedo;

#include "tonemap.glsl"

void main() {
    vec3 color = texture(denoisedImage, texCoords).rgb * texture(gbufferAlbedo, texCoords).rgb;
	
	color = ACESFitted(color);
	color = sRGB(color);

	fragColor.rgb = color;
}