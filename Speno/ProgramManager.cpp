#include "ProgramManager.h"

#include <iostream>

#include "SharedEngineData.h"

ProgramManager ProgramManager::Instance;

void ProgramManager::drawInOrder() {
	for (const std::string order_member : m_draw_order)
		get(order_member)->draw();
}

void ProgramManager::resizeFramebuffers(int width, int height) {
	for (const auto& program_pair : m_programs) {
		Program* program = program_pair.second.get();
		program->resizeFBO(width, height);
	}
}

Program* ProgramManager::get(const std::string& name) {
	auto it = m_programs.find(name);

	if (it == m_programs.end())
		throw std::runtime_error("Program " + name + " not found.");

	return it->second.get();
}

Program* ProgramManager::add(const std::string& name) {
	m_programs.emplace(name, std::make_unique<Program>());
	m_draw_order.push_back(name);
	return m_programs.at(name).get();
}

void ProgramManager::remove(const std::string& name) {
	m_programs.erase(name);
	m_draw_order.erase(std::remove(m_draw_order.begin(), m_draw_order.end(), name));
}

void ProgramManager::enableAll() {
	for (const auto& program_pair : m_programs)
		program_pair.second->enable();
}

void ProgramManager::disableAll() {
	for (const auto& program_pair : m_programs)
		program_pair.second->disable();
}