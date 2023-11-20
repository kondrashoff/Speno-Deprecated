#pragma once

#include "Utils.h"

#define SKY_TYPE_BLACK     0
#define SKY_TYPE_DEFAULT   1
#define SKY_TYPE_REALISTIC 2

struct Sky {
	int type = SKY_TYPE_DEFAULT;
	alignas(16) Vector3 sun_direction;
	int sun_quality_i = 16;
	int sun_quality_j = 8;
	float pitch;
	float yaw;

	Sky() {
		sun_direction = Vector3(0, 1, 0);
	}

	Sky(float other_pitch, float other_yaw) {
		pitch = other_pitch;
		yaw = other_yaw;

		type = SKY_TYPE_REALISTIC;

		Vector3 direction;

		float pitch_radians = degrees_to_radians(pitch);
		float yaw_radians = degrees_to_radians(yaw);

		float sin_pitch = std::sin(pitch_radians);
		float cos_pitch = std::cos(pitch_radians);

		float sin_yaw = std::sin(yaw_radians);
		float cos_yaw = std::cos(yaw_radians);

		direction.x = cos_yaw * cos_pitch;
		direction.y = sin_pitch;
		direction.z = sin_yaw * cos_pitch;

		sun_direction = normalize(direction);
	}
};