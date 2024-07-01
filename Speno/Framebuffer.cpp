#include "Framebuffer.h"

#include <iostream>

#include "SharedEngineData.h"

void Framebuffer::init() {
	glCreateFramebuffers(1, &m_fbo);
	m_is_initialized = true;
}

void Framebuffer::resize(int width, int height, float resolution_scale) {
	if (!m_is_initialized)
		return;

	width = static_cast<int>(float(width) * resolution_scale);
	height = static_cast<int>(float(height) * resolution_scale);

	for (auto& texture : m_textures) {
		FBOtexture& tex = texture.second;
		tex.recreate(width, height);
		glNamedFramebufferTexture(m_fbo, tex.getAttachment(), tex.getID(), 0);
	}
}

void Framebuffer::addAttachment(const std::string& attachment_name, GLenum internalformat, TextureParameters params) {
	if(!m_is_initialized)
		throw std::runtime_error("Framebuffer is not initialized!");

	if (m_textures.find(attachment_name) != m_textures.end())
		throw std::runtime_error("Framebuffer already has attachment with name " + attachment_name);

	GLenum current_attachment = GL_COLOR_ATTACHMENT0 + m_attachments.size();
	m_attachments.push_back(current_attachment);

	FBOtexture attachment_texture;
	attachment_texture.initEmpty(0, 0, internalformat, params);
	attachment_texture.setAttachment(current_attachment);
	m_textures.emplace(attachment_name, attachment_texture);

	glNamedFramebufferTexture(m_fbo, current_attachment, m_textures[attachment_name].getID(), 0);
	
	size_t size = m_attachments.size();
	glNamedFramebufferDrawBuffers(m_fbo, size, m_attachments.data());
	check();
}

Texture* Framebuffer::getAttachment(const std::string& attachment_name) {
	auto it = m_textures.find(attachment_name);

	if (it == m_textures.end())
		throw std::runtime_error("Attachment texture " + attachment_name + " not found!");

	return &it->second;
}

void Framebuffer::check() {
	if (!m_is_initialized) 
		return;

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer error " + status);
}

void Framebuffer::bind() {
	if (m_is_initialized)
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind() {
	if (m_is_initialized)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
	if(m_is_initialized)
		glDeleteFramebuffers(1, &m_fbo);
}