#include "FlagManager.h"

FlagSystem FlagSystem::Instance;

bool FlagManager::disableIfEnabled(unsigned int flag) {
	if (isEnabled(flag)) {
		disable(flag);
		return true;
	}
	
	return false;
}

void FlagManager::set(unsigned int flag, bool enable) {
	if (enable) this->enable(flag);
	else disable(flag);
}

void FlagManager::enable(unsigned int flag) {
	m_flags |= flag;
}

void FlagManager::disable(unsigned int flag) {
	m_flags &= ~flag;
}

void FlagManager::toggle(unsigned int flag) {
	m_flags ^= flag;
}

bool FlagManager::isEnabled(unsigned int flag) {
	return m_flags & flag;
}