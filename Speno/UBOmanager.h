#pragma once

#include <string>
#include <memory>
#include <map>

#include "Buffer.h"

class UBOmanager {
public:
	static UBOmanager Instance;

	Buffer* create(std::string name, void* data, size_t size, GLenum usage = GL_STATIC_DRAW);
	void bind(std::string buffer_name, std::string program_name);
	void bind(std::string buffer_name, GLuint program_id);
	Buffer* add(std::string name);
	Buffer* get(std::string name);
	void update();

private:
	std::map<std::string, std::unique_ptr<Buffer>> m_ubos;
	int m_current_free_binding = 0;
};