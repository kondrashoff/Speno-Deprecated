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

				double mountains = 245.0 * std::pow(0.5 * perlin.octave2D_01(dx * 0.01, dz * 0.01, 8) + 0.5, 2.0);
				double plains = 10.0 * (0.5 * perlin.noise2D_01(dx * 0.03, dz * 0.03) + 0.5);
				double mixture = std::pow(0.5 * perlin.noise2D_01(dx * 0.003, dz * 0.003) + 0.5, 3.0);
				double inv_mixture = (1.0 - mixture);
				int height = int(inv_mixture * plains + mixture * mountains);

				for (int y = 0; y < height; y++) {
					blocks[x][y][z] = 1;
				}

				blocks[x][height][z] = 3;
				blocks[x][height+1][z] = 2;

				for (int y = height + 2; y < 255; y++) {
					blocks[x][y][z] = 0;
				}

				/*for (int y = 0; y < 255; y++) {
					double dy = static_cast<double>(y);
					double noise = 0.5 * perlin.octave3D_01(dx * 0.03, dy * 0.03, dz * 0.03, 4) + 0.5 * perlin.octave3D_01(dx * 0.1, dy * 0.1, dz * 0.1, 2) + std::exp((y - 127.5) * 0.01) - 1.0;
					
					if (noise < 0.4) {
						if (false && noise > 0.35 && rand() < 128) {
							blocks[x][y][z] = 3;
						}
						else {
							if (noise > 0.38) blocks[x][y][z] = 2;
							else blocks[x][y][z] = 1;
						}
					}
					else {
						blocks[x][y][z] = 0;
					}
				}*/
			}
		}
	}
};

struct BlockWorld {
	std::vector<Chunk> chunks;
	int generation_distance = 64;

	BlockWorld() = default;
};