#version 460 core

out vec4 fragColor;
in vec2 texCoords;

uniform sampler2D postprocessedFrame;

void main() {
	fragColor = texture(postprocessedFrame, texCoords);
}