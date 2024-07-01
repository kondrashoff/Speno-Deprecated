#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 fragColor;
in vec2 texCoords;

layout(bindless_sampler) uniform sampler2D logo_texture;

#include "shared_engine_data.glsl"

void main() {
	vec2 uv = texCoords;
	vec2 resolution = vec2(width, height);
	vec2 logo_texture_size = textureSize(logo_texture, 0).xy;

	float logo_aspect_ratio = logo_texture_size.x / logo_texture_size.y;
	float screen_aspect_ratio = resolution.x / resolution.y;

	if (logo_aspect_ratio > screen_aspect_ratio) {
		float scale_factor = screen_aspect_ratio / logo_aspect_ratio;
		uv.x = (uv.x - 0.5) * scale_factor + 0.5;
	} else {
		float scale_factor = logo_aspect_ratio / screen_aspect_ratio;
		uv.y = (uv.y - 0.5) * scale_factor + 0.5;
	}

	vec3 color = texture(logo_texture, uv).rgb;

	uv = uv * 5.0 - vec2(3.188, 2.438);
	if(uv.x < -1.0 || uv.x > 1.0 || uv.y < -1.0 || uv.y > 1.0) {
		fragColor = vec4(color, 1.0);
		return;
	}

	float circle1 = smoothstep(0.0, 1.0, 1.0 - length(uv * 2.6));
    float circle2 = smoothstep(0.0, 0.6, 1.0 - length(uv * 3.0));
    float circle3 = 5.0 * (circle1 - circle2);
    float st = time * 10.0;
	uv.x = uv.x - sin(st) * 0.9;
	uv.y = 1.7 * uv.y - cos(st) * 0.7;
    circle3 -= smoothstep(0.0, 1.0, 1.0 - length(uv * 0.8));
    
    vec3 loader = vec3(circle3) * vec3(0.94, 0.33, 0.23);
    loader += vec3(smoothstep(0.2, 0.7, loader)) * vec3(1.0, 1.0, 0.0);
    loader += vec3(smoothstep(0.4, 0.55, loader));
	loader = max(loader, 0.0);
    
    fragColor = vec4(color + (1.0 - color) * loader, 1.0);
}