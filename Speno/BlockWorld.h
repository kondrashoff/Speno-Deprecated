#pragma once

#include <cmath>
#include <cstdlib>

#include "PerlinNoise.h"

struct Chunk {
	int blocks[16][255][16];

	Chunk() = default;

	void generate(unsigned int seed, int start_x, int start_z) {
		const siv::PerlinNoise::seed_type noise_seed = seed;
		const siv::PerlinNoise perlin{ noise_seed };

		for (int x = 0; x < 16; x++) {
			double dx = static_cast<double>(start_x + x);
			for (int z = 0; z < 16; z++) {
				double dz = static_cast<double>(start_z + z);

				double dm = 1.0 - std::abs(perlin.octave2D(dx * 0.002, dz * 0.002, 8));
				       dm = std::min(dm, 1.0 - std::abs(perlin.octave2D((dx + 93.981) * 0.004, (dz - 893.82) * 0.004, 8)));
				       dm = std::min(dm, 1.0 - std::abs(perlin.octave2D((dx - 342.45) * 0.008, (dz + 128.18) * 0.008, 8)));
				double mountains = 255.0 * dm;

				double n1 = perlin.octave2D( dx           * 0.003,  dz           * 0.003, 8);
				double n2 = perlin.octave2D((dx + 93.981) * 0.005, (dz - 893.82) * 0.005, 8);
				double n3 = perlin.octave2D((dx - 342.45) * 0.01,  (dz + 128.18) * 0.01,  8);

				double v1 = 1.0 - pow(std::abs(std::sin(0.003 * dz + 1.2 * n1 + dm)), 3.0);
				double v2 = 1.0 - pow(std::abs(std::sin(0.005 * dz + 1.2 * n2 + dm)), 3.0);
				double v3 = 1.0 - pow(std::abs(std::sin(0.01  * dz + 1.2 * n3 + dm)), 3.0);

				double dc = v1;
				dc = std::min(dc, v2);
				dc = std::min(dc, v3);

				double canyons = 60.0 * dc;
				int height = std::clamp(int((canyons + mountains) / 2.0), 2, 255);

				for (int y = 0; y < height - 2; y++) {
					blocks[x][y][z] = 1;
				}

				blocks[x][height-2][z] = 3;
				blocks[x][height-1][z] = 2;

				for (int y = height; y < 255; y++) {
					blocks[x][y][z] = 0;
				}
			}
		}
	}
};

struct BlockWorld {
	std::vector<Chunk> chunks;
	int generation_distance = 64;

	BlockWorld() = default;
};