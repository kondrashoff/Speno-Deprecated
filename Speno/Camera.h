#pragma once

class Camera {
private:

    struct CameraUniform {
        alignas(16) glm::vec3 view_u, view_v, view_w;
        alignas(16) glm::vec3 lookfrom;
        alignas(16) glm::vec3 view_prev_u, view_prev_v, view_prev_w;
        alignas(16) glm::vec3 lookfrom_prev;
        float tan_half_fov;
        float tan_half_fov_prev;
        unsigned int max_depth;
    };

public:

    void init(glm::vec3 lookfrom = glm::vec3(0), float fov = 60.0, float speed = 1.0, int max_depth = 4, float pitch = 0.0, float yaw = 0.0) {
        this->lookfrom = lookfrom;
        this->speed = speed;
        this->max_depth = max_depth;
        this->fov = fov;
        this->pitch = pitch;
        this->yaw = yaw;
        is_lookat_set = false;

        buildFromRotations();
        buildViewMatrix();

        CameraUniform uniform = getUniform();

        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CameraUniform), &uniform, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    }

    void init(glm::vec3 lookfrom, glm::vec3 lookat, float fov = 60.0, float speed = 1.0, int max_depth = 4) {
        this->lookfrom = lookfrom;
        this->lookat = lookat;
        this->speed = speed;
        this->max_depth = max_depth;
        this->fov = fov;
        is_lookat_set = true;

        pitch = 0.0f;
        yaw = 0.0f;

        this->lookdir = glm::normalize(lookat - lookfrom);

        CameraUniform uniform = getUniform();

        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CameraUniform), &uniform, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    }

    void update() {
        if (is_lookat_set) lookdir = glm::normalize(lookat - lookfrom);
        else buildFromRotations();
        buildViewMatrix();

        tan_half_fov = glm::tan(glm::radians(fov) / 2.0f);

        CameraUniform uniform = getUniform();

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CameraUniform), &uniform);
    }

    void updateLookAt(glm::vec3 lookat) {
        this->lookat = lookat;
        is_lookat_set = true;
    }

    void stopLookAt() {
        is_lookat_set = false;
    }

    inline void beforeMovement() {
        view_matrix_previous = view_matrix;
        lookfrom_previous = lookfrom;
        tan_half_fov_previous = tan_half_fov;
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

    inline glm::vec3 getCameraPosition() {
        return lookfrom;
    }

private:

    void buildViewMatrix() {
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, lookdir));
        up = glm::normalize(glm::cross(lookdir, right));
        view_matrix = glm::mat3(right, up, lookdir);
    }

    void buildFromRotations() {
        glm::vec3 direction;

        float pitch_radians = glm::radians(pitch);
        float yaw_radians = glm::radians(yaw);

        float sin_pitch = glm::sin(pitch_radians);
        float cos_pitch = glm::cos(pitch_radians);

        float sin_yaw = glm::sin(yaw_radians);
        float cos_yaw = glm::cos(yaw_radians);

        direction.x = cos_yaw * cos_pitch;
        direction.y = sin_pitch;
        direction.z = sin_yaw * cos_pitch;

        lookdir = glm::normalize(direction);
    }

    inline CameraUniform getUniform() {
        CameraUniform uniform;
        uniform.view_u = view_matrix[0];
        uniform.view_v = view_matrix[1];
        uniform.view_w = view_matrix[2];
        uniform.view_prev_u = view_matrix_previous[0];
        uniform.view_prev_v = view_matrix_previous[1];
        uniform.view_prev_w = view_matrix_previous[2];
        uniform.lookfrom = lookfrom;
        uniform.lookfrom_prev = lookfrom_previous;
        uniform.tan_half_fov = tan_half_fov;
        uniform.tan_half_fov_prev = tan_half_fov_previous;
        uniform.max_depth = (unsigned int)max_depth;
        return uniform;
    }

public:

    float pitch;
    float yaw;
    float speed;
    float fov;
    int max_depth;
    bool is_lookat_set;

private:

    float tan_half_fov;
    float tan_half_fov_previous;
    GLuint ssbo;
    glm::vec3 lookfrom;
    glm::vec3 lookfrom_previous;
    glm::vec3 lookdir;
    glm::vec3 lookat;
    glm::mat3 view_matrix;
    glm::mat3 view_matrix_previous;
};