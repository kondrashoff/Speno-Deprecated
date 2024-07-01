#pragma once

#include "Flags.h"

class FlagManager {
public:
	FlagManager(unsigned int flags = 0) : m_flags(flags) {};

	void set(unsigned int flag, bool enable);
	void enable(unsigned int flag);
	void disable(unsigned int flag);
	void toggle(unsigned int flag);
	bool isEnabled(unsigned int flag);
	bool disableIfEnabled(unsigned int flag);

private:
	unsigned int m_flags;
};

struct FlagSystem {
	static FlagSystem Instance;
	
	FlagManager* window;
	FlagManager* camera;

	FlagSystem(FlagSystem const&) = delete;
	void operator=(FlagSystem const&) = delete;

private:
	FlagSystem() {}
};