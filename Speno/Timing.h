#pragma once

#include <stdint.h>

struct Timing {
	static Timing global;

	void start();
	void restart();
	void stop();

	void restartIfElapsed(float time);
	void accumulate();
	float getElapsed();

	uint32_t accumulated = 0;
	float time_elapsed = 0.0f;
	float average_elapsed = 0.0f;
	float highest_elapsed = 0.0f;

private:
	bool started = false;

	float highest_accumulation = 0.0f;

	uint64_t t0;
	uint64_t t1;
};