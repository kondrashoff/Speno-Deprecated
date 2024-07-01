#include "Buffer.h"

#include <iostream>

void Buffer::createAsUBO() {
	if(is_created)
		throw std::runtime_error("Buffer already created");

	glCreateBuffers(1, &id);
	target = GL_UNIFORM_BUFFER;
	is_created = true;
}

void Buffer::createAsUBO(void* data, size_t size, GLenum usage) {
	if (is_created)
		throw std::runtime_error("Buffer already created");

	glCreateBuffers(1, &id);
	target = GL_UNIFORM_BUFFER;
	is_created = true;

	if (data == nullptr && size != 0)
		throw std::runtime_error("Data is nullptr but size is not 0");

	glBindBuffer(target, id);
	glNamedBufferData(id, size, data, usage);
	this->data = data;
	this->size = size;
	this->usage = usage;
}

void Buffer::createAsSSBO() {
	if (is_created)
		throw std::runtime_error("Buffer already created");

	glCreateBuffers(1, &id);
	target = GL_SHADER_STORAGE_BUFFER;
	is_created = true;
}

void Buffer::createAsSSBO(void* data, size_t size, GLenum usage) {
	if (is_created)
		throw std::runtime_error("Buffer already created");

	glCreateBuffers(1, &id);
	target = GL_SHADER_STORAGE_BUFFER;
	is_created = true;

	if (data == nullptr && size != 0)
		throw std::runtime_error("Data is nullptr but size is not 0");

	glBindBuffer(target, id);
	glNamedBufferData(id, size, data, usage);
	this->data = data;
	this->size = size;
	this->usage = usage;
}

void Buffer::setData(void* data, size_t size, GLenum usage) {
	if ((data == nullptr && size != 0) || !is_created)
		throw std::runtime_error("Failed to set data");

	glBindBuffer(target, id);
	glNamedBufferData(id, size, data, usage);
	this->data = data;
	this->size = size;
	this->usage = usage;
}

void Buffer::bindBase(int binding) {
	if (!is_created)
		throw std::runtime_error("Failed to bind buffer base");

	glBindBuffer(target, id);
	glBindBufferBase(target, binding, id);
	this->binding = binding;
}

void Buffer::update(void* data, size_t offset, size_t size) {
	if(binding == -1)
		throw std::runtime_error("Buffer not bound");

	glNamedBufferSubData(id, offset, size, data);
}

void Buffer::update() {
	if(binding == -1)
		throw std::runtime_error("Buffer not bound");

	if (usage == GL_STATIC_DRAW) return;

	glNamedBufferSubData(id, 0, size, data);
}

Buffer::~Buffer() {
	glDeleteBuffers(1, &id);
}