#pragma once

#include <string>

#include <glm/glm.hpp>
using namespace glm;

#include "Texture.h"

struct UniformMaterial {
    alignas(16) vec3 diffuse_color = vec3(0.0f);
    alignas(16) vec3 emmision_color = vec3(0.0f);
    float roughness = 0.0f;

    GLuint64 diffuse_tex_handle;
    alignas(16) vec3 diffuse_offset = vec3(0.0f);
    alignas(16) vec3 diffuse_scale = vec3(1.0f);
    alignas(4) bool use_diffuse_tex = false;
    
    GLuint64 emissive_tex_handle;
    alignas(16) vec3 emissive_offset = vec3(0.0f);
    alignas(16) vec3 emissive_scale = vec3(1.0f);
    alignas(4) bool use_emissive_tex = false;

	GLuint64 normal_tex_handle;
	alignas(16) vec3 normal_offset = vec3(0.0f);
	alignas(16) vec3 normal_scale = vec3(1.0f);
	alignas(4) bool use_normal_tex = false;

	GLuint64 roughness_tex_handle;
    alignas(16) vec3 roughness_offset = vec3(0.0f);
    alignas(16) vec3 roughness_scale = vec3(1.0f);
    alignas(4) bool use_roughness_tex = false;
};

struct Material : UniformMaterial {
	std::string name;
    Texture diffuse_texture;
    Texture emissive_texture;
    Texture normal_texture;
    Texture roughness_texture;
};