#include "SSBOmanager.h"

#include <iostream>
#include <string>

#include "ProgramManager.h"

SSBOmanager SSBOmanager::Instance;

Buffer* SSBOmanager::create(int binding, void* data, size_t size, GLenum usage) {
	m_ssbos.emplace(binding, std::make_unique<Buffer>());

	Buffer* ssbo = m_ssbos.at(binding).get();
	ssbo->createAsSSBO(data, size, usage);
	ssbo->bindBase(binding);

	return ssbo;
}

Buffer* SSBOmanager::createOrSet(int binding, void* data, size_t size, GLenum usage) {
	auto it = m_ssbos.find(binding);

	if (it != m_ssbos.end()) {
		Buffer* ssbo = it->second.get();
		ssbo->setData(data, size, usage);
		return ssbo;
	}
	else {
		return create(binding, data, size, usage);
	}
}

Buffer* SSBOmanager::add(int binding) {
	m_ssbos.emplace(binding, std::make_unique<Buffer>());

	Buffer* ssbo = m_ssbos.at(binding).get();
	ssbo->createAsSSBO();
	ssbo->bindBase(binding);

	return m_ssbos.at(binding).get();
}

Buffer* SSBOmanager::get(int binding) {
	auto it = m_ssbos.find(binding);

	if (it != m_ssbos.end())
		return it->second.get();
	else
		throw std::runtime_error("SSBO at " + std::to_string(binding) + " binding not found");
}

void SSBOmanager::update() {
	for (const auto& buffer_pair : m_ssbos)
		buffer_pair.second.get()->update();
}