#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 texCoords;
uniform vec2 resolution;

layout(location = 0) out vec3 gbufferCurrentDenoised;
layout(location = 1) out vec4 gbufferCurrentPosition;
layout(location = 2) out vec2 gbufferCurrentMoment;
layout(location = 3) out vec4 gbufferCurrentGodrays;
layout(location = 4) out vec4 gbufferCurrentReservoir1;
layout(location = 5) out vec4 gbufferCurrentReservoir2;

layout(bindless_sampler) uniform sampler2D godraysTexture;
layout(bindless_sampler) uniform sampler2D denoisedTexture;
layout(bindless_sampler) uniform sampler2D positionTexture;
layout(bindless_sampler) uniform sampler2D momentTexture;
layout(bindless_sampler) uniform sampler2D reservoir1Texture;
layout(bindless_sampler) uniform sampler2D reservoir2Texture;

void main() {
	gbufferCurrentDenoised = texture(denoisedTexture, texCoords).rgb;
	gbufferCurrentPosition = texture(positionTexture, texCoords);
	gbufferCurrentMoment = texture(momentTexture, texCoords).rg;
	gbufferCurrentGodrays = texture(godraysTexture, texCoords);
	gbufferCurrentReservoir1 = texture(reservoir1Texture, texCoords);
	gbufferCurrentReservoir2 = texture(reservoir2Texture, texCoords);
}