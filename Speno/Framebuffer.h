#pragma once

#include <map>
#include <vector>
#include <string>

#include <glad/glad.h>

#include "Texture.h"

class Framebuffer {
public:
	void init();
	void bind();
	void unbind();
	void check();

	void addAttachment(const std::string& attachment_name, GLenum internalformat = GL_RGBA32F, TextureParameters params = TextureParameters());
	Texture* getAttachment(const std::string& attachment_name);
	void resize(int width, int height, float resolution_scale);

	~Framebuffer();

private:
	GLuint m_fbo;
	std::map<std::string, FBOtexture> m_textures;
	std::vector<GLenum> m_attachments;
	bool m_is_initialized = false;
};