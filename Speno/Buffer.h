#pragma once

#include <glad/glad.h>

struct Buffer {
	GLuint id;
	int binding = -1;

	size_t size;
	void* data;
	GLenum usage;

	Buffer() = default;

	Buffer(GLuint id, int binding, void* data, size_t size, GLenum usage) :
		id(id), binding(binding), size(size), data(data), usage(usage), is_created(true) {}

	void createAsUBO();
	void createAsUBO(void* data, size_t size, GLenum usage = GL_STATIC_DRAW);

	void createAsSSBO();
	void createAsSSBO(void* data, size_t size, GLenum usage = GL_STATIC_DRAW);

	void setData(void* data, size_t size, GLenum usage = GL_STATIC_DRAW);

	void bindBase(int binding);

	void update(void* data, size_t offset, size_t size);
	void update();

	~Buffer();

private:
	GLenum target;
	bool is_created = false;
};