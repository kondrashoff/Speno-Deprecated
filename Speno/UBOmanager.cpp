#include "UBOmanager.h"

#include <iostream>

#include "ProgramManager.h"

UBOmanager UBOmanager::Instance;

Buffer* UBOmanager::create(std::string name, void* data, size_t size, GLenum usage) {
	m_ubos.emplace(name, std::make_unique<Buffer>());

	Buffer* ubo = m_ubos.at(name).get();
	ubo->createAsUBO(data, size, usage);
	ubo->bindBase(m_current_free_binding++);
	
	return ubo;
}

void UBOmanager::bind(std::string buffer_name, std::string program_name) {
	GLuint program_id = ProgramManager::Instance.get(program_name)->getID();
	GLuint buffer_binding = get(buffer_name)->binding;

	GLuint block_index = glGetUniformBlockIndex(program_id, buffer_name.c_str());
	glUniformBlockBinding(program_id, block_index, buffer_binding);
}

void UBOmanager::bind(std::string buffer_name, GLuint program_id) {
	GLuint buffer_binding = get(buffer_name)->binding;
	GLuint block_index = glGetUniformBlockIndex(program_id, buffer_name.c_str());
	glUniformBlockBinding(program_id, block_index, buffer_binding);
}

Buffer* UBOmanager::add(std::string name) {
	m_ubos.emplace(name, std::make_unique<Buffer>());

	Buffer* ubo = m_ubos.at(name).get();
	ubo->createAsUBO();
	ubo->bindBase(m_current_free_binding++);

	return m_ubos.at(name).get();
}

Buffer* UBOmanager::get(std::string name) {
	auto it = m_ubos.find(name);

	if (it != m_ubos.end())
		return it->second.get();
	else
		throw std::runtime_error("UBO " + name + " not found");
}

void UBOmanager::update() {
	for (const auto& buffer_pair : m_ubos)
		buffer_pair.second.get()->update();
}