struct Ray {
	vec3 origin;
	vec3 direction;
    vec3 inverse_direction;
};

struct Camera {
    vec3 lookfrom;
    vec3 right;
    vec3 up;
    vec3 front;
    float tan_half_fov;
};

layout(std140) uniform Cameras {
    Camera camera;
    Camera old_camera;
};

Ray getRay(vec2 coords) {
    vec2 uv = 2.0 * coords - 1.0;
    uv.x *= resolution.x / resolution.y;
    uv *= camera.tan_half_fov;

    vec3 direction = camera.right * uv.x + camera.up * uv.y + camera.front;
    direction = normalize(direction);

    return Ray(camera.lookfrom, direction, vec3(0.0));
}

vec2 getPreviousUV(vec3 position) {
    vec3 direction = position - old_camera.lookfrom;
    float dp = old_camera.tan_half_fov * dot(direction, old_camera.front);

    vec2 uv;
    uv.y = dot(direction, old_camera.up) / dp;
    uv.x = dot(direction, old_camera.right) / dp;

    uv.x /= resolution.x / resolution.y;
    uv = 0.5 * uv + 0.5;

    return uv;
}

Ray getPreviousRay(vec2 coords) {
    vec2 uv = 2.0 * coords - 1.0;
    uv.x *= resolution.x / resolution.y;
    uv *= old_camera.tan_half_fov;

    vec3 direction = old_camera.right * uv.x + old_camera.up * uv.y + old_camera.front;
    direction = normalize(direction);

    return Ray(old_camera.lookfrom, direction, vec3(0.0));
}