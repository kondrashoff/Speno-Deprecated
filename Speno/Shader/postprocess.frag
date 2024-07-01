#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 fragColor;
in vec2 texCoords;

uniform vec2 resolution;

layout(bindless_sampler) uniform sampler2D denoisedTexture;
layout(bindless_sampler) uniform sampler2D albedoTexture;
layout(bindless_sampler) uniform sampler2D godraysTexture;
layout(bindless_sampler) uniform sampler2D positionTexture;

#include "defines.glsl"
#include "scene.glsl"
#include "color_functions.glsl"

void main() {
    vec2 uv = vec2(1.0 - texCoords.x, texCoords.y);

    vec3 diffuse = texture(denoisedTexture, uv).rgb;
    vec3 albedo = texture(albedoTexture, uv).rgb;
    vec3 color = diffuse * albedo;

    /*float dist = texelFetch(positionTexture, ivec2(uv * textureSize(positionTexture, 0)), 0).w;
    AABB root = getBVHRootBounds();
	float size = length(root.maximum - root.minimum);

	float density = size == 0.0 ? 1.0 : 1.0 / size;
    vec3 godrays = texture(godraysTexture, uv).rgb;
    color = dist < 0.0 ? color + godrays : mix(godrays, color, exp(-1.0 / density * dist));*/

    //color /= 1.0 + color;
    //color = reinhard(color, 11.3);
    //color = reinhard(color, 34.6);
    //color = ACES(color);
    color = uncharted2(color);
    color = sRGB(color);

    fragColor = vec4(color, 1.0);
}