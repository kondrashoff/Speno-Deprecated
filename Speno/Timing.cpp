#include "Timing.h"

#include <iostream>
#include <chrono>

Timing Timing::global;

void Timing::start() {
	auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
	t0 = std::chrono::duration_cast<std::chrono::microseconds>(time).count();

	average_elapsed = 0.0f;
	highest_elapsed = 0.0f;
	highest_accumulation = 0.0f;
	time_elapsed = 0.0f;
	accumulated = 0U;
	started = true;
}

void Timing::restart() {
	auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
	t0 = std::chrono::duration_cast<std::chrono::microseconds>(time).count();
	average_elapsed = time_elapsed / static_cast<float>(accumulated);
	highest_elapsed = highest_accumulation;

	time_elapsed = 0.0f;
	highest_accumulation = 0.0f;
	accumulated = 0U;
	started = true;
}

void Timing::stop() {
	auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
	t1 = std::chrono::duration_cast<std::chrono::microseconds>(time).count();

	started = false;
}

void Timing::restartIfElapsed(float time) {
	if (time_elapsed > time) restart();
}

void Timing::accumulate() {
	if (!started) return;

	auto t1_time = std::chrono::high_resolution_clock::now().time_since_epoch();
	t1 = std::chrono::duration_cast<std::chrono::microseconds>(t1_time).count();
	
	float time = 0.001f * (t1 - t0);

	highest_accumulation = std::max(highest_accumulation, time);
	time_elapsed += time;
	accumulated++;

	auto t0_time = std::chrono::high_resolution_clock::now().time_since_epoch();
	t0 = std::chrono::duration_cast<std::chrono::microseconds>(t0_time).count();
}

float Timing::getElapsed() {
	if (started) return time_elapsed;
	
	return 0.001f * (t1 - t0);
}
