#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

struct TextureParameters {
	GLint wrap_s = GL_REPEAT;
	GLint wrap_t = GL_REPEAT;
	GLint min_filter = GL_LINEAR;
	GLint mag_filter = GL_LINEAR;
};

class Texture {
public:
	void initEmpty(int width, int height, GLenum internalformat = GL_RGBA8, const TextureParameters& params = TextureParameters());
	void initWhite(int width, int height, GLenum internalformat = GL_RGBA8, const TextureParameters& params = TextureParameters());
	bool initFromFile(const std::string& filepath, const TextureParameters& params = TextureParameters());
	bool initFromFileHDR(const std::string& filepath, const TextureParameters& params = TextureParameters());
	
	void initLayered(int width, int height, int layers, const TextureParameters& params = TextureParameters());
	void addLayerFromFile(const std::string& filepath);
	
	void setParameters(GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter);

	void bind(const std::string& uniform_name, const std::string& program_name);
	GLuint64 bindAndGet();
	void rebind();
	void unbind();

	void getSizes(int* width, int* height);
	GLuint getID();

	~Texture();

private:
	struct Binding {
		std::string uniform_name;
		std::string program_name;
	};

protected:
	int m_width;
	int m_height;

	GLuint m_id;
	GLenum m_internalformat;
	TextureParameters m_params;

private:
	bool m_is_layered = false;
	int m_layer_pointer = 0;
	int m_layers_total;

	bool m_is_binded = false;
	GLuint64 m_handle;
	std::vector<Binding> m_bindings;
};

class FBOtexture : public Texture {
public:
	void recreate(int width, int height);
	void setAttachment(GLenum attachment);
	GLenum getAttachment();

	void initWhite(int width, int height, GLenum internalformat, const TextureParameters& params) = delete;
	void initFromFile(const std::string& filepath, const TextureParameters& params) = delete;
	void initFromFileHDR(const std::string& filepath, const TextureParameters& params) = delete;
	void setParameters(GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter) = delete;
	void addLayerFromFile(const std::string& filepath) = delete;

	FBOtexture() = default;
	FBOtexture(const Texture& texture, GLenum attachment)
		: Texture(texture), m_attachment(attachment) {}

private:
	GLenum m_attachment;
};