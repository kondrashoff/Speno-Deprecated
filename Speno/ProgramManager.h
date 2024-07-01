#pragma once

#include <map>
#include <memory>

#include "Program.h"

class ProgramManager {
public:
	static ProgramManager Instance;

	ProgramManager(ProgramManager const&) = delete;
	void operator=(ProgramManager const&) = delete; 

	void drawInOrder();
	Program* add(const std::string& name);
	Program* get(const std::string& name);
	void remove(const std::string& name);

	void enableAll();
	void disableAll();

	void resizeFramebuffers(int width, int height);

private:
	ProgramManager() {}

private:
	std::map<std::string, std::unique_ptr<Program>> m_programs;
	std::vector<std::string> m_draw_order;
};