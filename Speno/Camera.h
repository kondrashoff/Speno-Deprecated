#pragma once

#include "Utils.h"

struct Camera {
    //alignas(16) Vector3 lookfrom = Vector3(145.038, 106.119, 62.046);
    alignas(16) Vector3 lookfrom = Vector3(129.218, 119.915, 56.6934);
    alignas(16) Vector3 lookdir = Vector3(1, 0, 0);

    //float pitch = -9.08338f;
    float pitch = -2.58337f;
    //float yaw = -699.914f;
    float yaw = -387.234f;
    
    float fov = 60.0f;
    float speed = 10.0f;

    unsigned int max_depth = 2;
    unsigned int samples_per_pixel = 1;

    void buildFromRotations() {
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

        lookdir = normalize(direction);
    }

    void stepForward(float delta_time) {
        lookfrom.x += speed * delta_time * lookdir.x;
        lookfrom.y += speed * delta_time * lookdir.y;
        lookfrom.z += speed * delta_time * lookdir.z;
    }

    void stepBack(float delta_time) {
        lookfrom.x -= speed * delta_time * lookdir.x;
        lookfrom.y -= speed * delta_time * lookdir.y;
        lookfrom.z -= speed * delta_time * lookdir.z;
    }

    void stepLeft(float delta_time) {
        lookfrom.x -= speed * delta_time * lookdir.z;
        lookfrom.z += speed * delta_time * lookdir.x;
    }

    void stepRight(float delta_time) {
        lookfrom.x += speed * delta_time * lookdir.z;
        lookfrom.z -= speed * delta_time * lookdir.x;
    }

    void stepUp(float delta_time) {
        lookfrom.y += speed * delta_time;
    }

    void stepDown(float delta_time) {
        lookfrom.y -= speed * delta_time;
    }
};