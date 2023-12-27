#version 460 core

layout(location = 0) out vec4 outGbufferDiffuse;
layout(location = 1) out vec4 outDenoisedImage;
layout(location = 3) out vec4 outGbufferPosition;
out vec4 fragColor;
in vec2 texCoords;

uniform sampler2D gbufferDiffuse;
uniform sampler2D gbufferNormal;
uniform sampler2D gbufferPosition;

uniform bool use_pathtracing_ambient_occlusion;
uniform bool use_pathtracing;

#include "denoising_functions.glsl"

void main() {
	vec4 diff = texture(gbufferDiffuse, texCoords);
	if(use_pathtracing_ambient_occlusion && !use_pathtracing) {
		diff.rgb = reBlur(1.0);
		outDenoisedImage.rgb = diff.rgb;
	}
	else outDenoisedImage.rgb = texture(gbufferDiffuse, texCoords).rgb;
	
	outGbufferDiffuse = diff;
	outGbufferPosition = texture(gbufferPosition, texCoords);
}