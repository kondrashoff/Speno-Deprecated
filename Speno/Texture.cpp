#include "Texture.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ProgramManager.h"
#include "Console.h"

bool Texture::initFromFile(const std::string& filepath, const TextureParameters& params) {
	int width, height, num_channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &num_channels, 0);
	
	if (!data) {
		Console::Instance.push("Failed to load image: " + filepath);
		return false;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	switch (num_channels) {
	case 1:
		initEmpty(width, height, GL_R8, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
		break;

	case 2:
		initEmpty(width, height, GL_RG8, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RG, GL_UNSIGNED_BYTE, data);
		break;

	case 3:
		initEmpty(width, height, GL_RGB8, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
		break;

	case 4:
		initEmpty(width, height, GL_RGBA8, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	stbi_image_free(data);
	return true;
}

bool Texture::initFromFileHDR(const std::string& filepath, const TextureParameters& params) {
	int width, height, num_channels;
	stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(filepath.c_str(), &width, &height, &num_channels, 0);

	if (!data) {
		Console::Instance.push("Failed to load HDR image: " + filepath);
		return false;
	}

	// If for some reason it crashes, then add:
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	// like in initFromFile()

	switch (num_channels) {
	case 1:
		initEmpty(width, height, GL_R32F, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RED, GL_FLOAT, data);
		break;

	case 2:
		initEmpty(width, height, GL_RG32F, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RG, GL_FLOAT, data);
		break;

	case 3:
		initEmpty(width, height, GL_RGB32F, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);
		break;

	case 4:
		initEmpty(width, height, GL_RGBA32F, params);
		glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);
		break;
	}

	stbi_image_free(data);
	return true;
}

void Texture::initEmpty(int width, int height, GLenum internalformat, const TextureParameters& params) {
	glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
	glTextureStorage2D(m_id, 1, internalformat, width, height);

	glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, params.wrap_s);
	glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, params.wrap_t);
	glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, params.min_filter);
	glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, params.mag_filter);

	m_internalformat = internalformat;
	m_width = width;
	m_height = height;
	m_params = params;
}

void Texture::initWhite(int width, int height, GLenum internalformat, const TextureParameters& params) {
	initEmpty(width, height, internalformat, params);

	int size = m_width * m_height * 4;
	float* white = new float[size];
	for (int i = 0; i < size; i++)
		white[i] = 1.0f;

	glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, GL_RGBA, GL_FLOAT, white);
}

void Texture::initLayered(int width, int height, int layers, const TextureParameters& params) {
	m_internalformat = GL_RGBA16;

	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_id);
	glTextureStorage3D(m_id, 1, m_internalformat, width, height, layers);

	glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, params.wrap_s);
	glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, params.wrap_t);
	glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, params.min_filter);
	glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, params.mag_filter);

	m_is_layered = true;
	m_layers_total = layers;
	m_width = width;
	m_height = height;
	m_params = params;
}

// TODO: make this function return bool
void Texture::addLayerFromFile(const std::string& filepath) {
	if(!m_is_layered)
		throw std::runtime_error("Texture is not layered");

	int width, height, num_channels;
	stbi_set_flip_vertically_on_load(true);
	stbi_us* data = stbi_load_16(filepath.c_str(), &width, &height, &num_channels, 0);

	if (width != m_width || height != m_height)
		throw std::runtime_error("Image size mismatch: " + filepath);

	if (!data)
		throw std::runtime_error("Failed to load image: " + filepath);
	
	int current_layer = m_layer_pointer++;
	switch (num_channels) {
	case 1:
		glTextureSubImage3D(m_id, 0, 0, 0, current_layer, m_width, m_height, 1, GL_RED, GL_UNSIGNED_SHORT, data);
		break;
	case 2:
		glTextureSubImage3D(m_id, 0, 0, 0, current_layer, m_width, m_height, 1, GL_RG, GL_UNSIGNED_SHORT, data);
		break;
	case 3:
		glTextureSubImage3D(m_id, 0, 0, 0, current_layer, m_width, m_height, 1, GL_RGB, GL_UNSIGNED_SHORT, data);
		break;
	case 4:
		glTextureSubImage3D(m_id, 0, 0, 0, current_layer, m_width, m_height, 1, GL_RGBA, GL_UNSIGNED_SHORT, data);
		break;
	}
	
	stbi_image_free(data);
}

void Texture::setParameters(GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter) {
	if(m_is_binded)
		throw std::runtime_error("You can't change texture parameters after texture is binded to GPU.");

	glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrap_s);
	glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrap_t);
	glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, min_filter);
	glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, mag_filter);

	m_params = TextureParameters(wrap_s, wrap_t, min_filter, mag_filter);
}

void Texture::bind(const std::string& uniform_name, const std::string& program_name) {
	Program* program = ProgramManager::Instance.get(program_name);
	glUseProgram(program->getID());

	if (!m_is_binded) {
		m_handle = glGetTextureHandleARB(m_id);
		glMakeTextureHandleResidentARB(m_handle);
		m_is_binded = true;
	}

	glUniformHandleui64ARB(glGetUniformLocation(program->getID(), uniform_name.c_str()), m_handle);
	m_bindings.emplace_back(uniform_name, program_name);
}

GLuint64 Texture::bindAndGet() {
	if (!m_is_binded) {
		m_handle = glGetTextureHandleARB(m_id);
		glMakeTextureHandleResidentARB(m_handle);
		m_is_binded = true;
	}

	return m_handle;
}

void Texture::unbind() {
	if (!m_is_binded)
		return;

	glMakeTextureHandleNonResidentARB(m_handle);
	m_is_binded = false;
}

void FBOtexture::recreate(int width, int height) {
	unbind();
	glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
	glTextureStorage2D(m_id, 1, m_internalformat, width, height);
	rebind();
}

void Texture::rebind() {
	if (m_bindings.empty()) return;

	for (Binding binding : m_bindings) {
		Program* program = ProgramManager::Instance.get(binding.program_name);
		glUseProgram(program->getID());

		if (!m_is_binded) {
			m_handle = glGetTextureHandleARB(m_id);
			glMakeTextureHandleResidentARB(m_handle);
			m_is_binded = true;
		}

		glUniformHandleui64ARB(glGetUniformLocation(program->getID(), binding.uniform_name.c_str()), m_handle);
	}
}

Texture::~Texture() {
	unbind();
	glDeleteTextures(1, &m_id);
}

void FBOtexture::setAttachment(GLenum attachment) {
	m_attachment = attachment;
}

GLenum FBOtexture::getAttachment() {
	return m_attachment;
}

void Texture::getSizes(int* width, int* height) {
	*width = m_width;
	*height = m_height;
}

GLuint Texture::getID() {
	return m_id;
}