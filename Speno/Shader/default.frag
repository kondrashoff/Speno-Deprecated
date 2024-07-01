#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 fragColor;
in vec2 texCoords;

uniform vec2 resolution;

void main() {
	fragColor = vec4(texCoords, 0.0, 1.0);
}