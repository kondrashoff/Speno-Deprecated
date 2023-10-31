#pragma once

#include "Utils.h"

struct Camera {
    alignas(16) Vector3 lookfrom = Vector3(260.0, 55.0, 0.0);
    alignas(16) Vector3 lookdir = Vector3(1, 0, 0);

    float pitch = 0.0;
    float yaw = 0.0;
    
    float fov = 60.0;
    float camera_speed = 22.5;

    int max_depth = 8;

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
        lookfrom.x += camera_speed * delta_time * lookdir.x;
        lookfrom.y += camera_speed * delta_time * lookdir.y;
        lookfrom.z += camera_speed * delta_time * lookdir.z;
    }

    void stepBack(float delta_time) {
        lookfrom.x -= camera_speed * delta_time * lookdir.x;
        lookfrom.y -= camera_speed * delta_time * lookdir.y;
        lookfrom.z -= camera_speed * delta_time * lookdir.z;
    }

    void stepLeft(float delta_time) {
        lookfrom.x -= camera_speed * delta_time * lookdir.z;
        lookfrom.z += camera_speed * delta_time * lookdir.x;
    }

    void stepRight(float delta_time) {
        lookfrom.x += camera_speed * delta_time * lookdir.z;
        lookfrom.z -= camera_speed * delta_time * lookdir.x;
    }

    void stepUp(float delta_time) {
        lookfrom.y += camera_speed * delta_time;
    }

    void stepDown(float delta_time) {
        lookfrom.y -= camera_speed * delta_time;
    }
};