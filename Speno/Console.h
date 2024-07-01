#pragma once

#include <vector>
#include <string>

constexpr int MAX_CONSOLE_SIZE = 512;

class Console {
public:
	static Console Instance;

	void push(const std::string& msg);
	void printToGUI();
	void setUseDebugConsole(bool value);

	Console(Console const&) = delete;
	void operator=(Console const&) = delete;
	
private:
	Console() {
		m_buffer.reserve(MAX_CONSOLE_SIZE);
		m_use_debug_console = true;
	}

private:
	std::vector<std::string> m_buffer;
	bool m_use_debug_console;
};