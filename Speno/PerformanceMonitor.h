#pragma once

struct Perfomance {
	clock_t begin;
	clock_t delta;
	int ms;

	void start() {
		begin = clock();
	}

	void stop() {
		delta = clock() - begin;
		ms = int((delta / (float)CLOCKS_PER_SEC) * 1000.0f);
	}

	Perfomance() = default;
};

static Perfomance global_prefomance_monitor;