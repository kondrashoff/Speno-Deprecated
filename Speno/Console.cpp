#include "Console.h"

#include <iostream>
#include <deque>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

Console Console::Instance;

void Console::push(const std::string& msg) {
	m_buffer.push_back(msg);

	if (m_buffer.size() > MAX_CONSOLE_SIZE) {
		std::deque<std::string> deque(m_buffer.begin(), m_buffer.end());
		deque.pop_front();
		m_buffer.assign(deque.begin(), deque.end());
	}

	if (m_use_debug_console) std::cout << msg << std::endl;
}

void Console::printToGUI() {
	for (const std::string& str : m_buffer) {
		ImGui::TextUnformatted(str.c_str());
	}
}

void Console::setUseDebugConsole(bool value) {
	m_use_debug_console = value;
}