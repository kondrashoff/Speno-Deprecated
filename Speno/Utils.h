#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>

constexpr float PI = 3.14159265358979323846f;

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3() = default;

    Vector3(float x, float y, float z)
        : x(x), y(y), z(z) {}

    Vector3(double x, double y, double z)
        : x(static_cast<float>(x)), y(static_cast<float>(y)), z(static_cast<float>(z)) {}

    Vector3(int x, int y, int z)
        : x(static_cast<float>(x)), y(static_cast<float>(y)), z(static_cast<float>(z)) {}

    Vector3 operator-() const { return Vector3(-x, -y, -z); }

    float operator[](int index) const {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;

        throw std::out_of_range("Index out of range in Vector3");
    }

    float& operator[](int index) {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;

        throw std::out_of_range("Index out of range in Vector3");
    }

    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    Vector3& operator+=(const Vector3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector3& operator*=(float t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vector3& operator/=(float t) {
        return *this *= 1.0f / t;
    }

    bool operator>(const Vector3& v) {
        return (x > v.x) && (y > v.y) && (z > v.z);
    }

    bool operator<(const Vector3& v) {
        return (x < v.x) && (y < v.y) && (z < v.z);
    }

    float length_squared() const {
        return x * x + y * y + z * z;
    }

    float length() const {
        return std::sqrt(length_squared());
    }
};

inline std::ostream& operator<<(std::ostream& out, const Vector3& v) {
    return out << v.x << ' ' << v.y << ' ' << v.z;
}

inline Vector3 operator+(const Vector3& u, const Vector3& v) {
    return Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline Vector3 operator-(const Vector3& u, const Vector3& v) {
    return Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
}

inline Vector3 operator*(const Vector3& u, const Vector3& v) {
    return Vector3(u.x * v.x, u.y * v.y, u.z * v.z);
}

inline Vector3 operator*(float t, const Vector3& v) {
    return Vector3(t * v.x, t * v.y, t * v.z);
}

inline Vector3 operator*(const Vector3& v, float t) {
    return t * v;
}

inline Vector3 operator/(Vector3 v, float t) {
    return (1 / t) * v;
}

inline bool operator<(const Vector3& v1, const Vector3& v2) {
    return (v1.x < v2.x) && (v1.y < v2.y) && (v1.z < v2.z);
}

inline Vector3 normalize(Vector3 v) {
    return v / v.length();
}

inline Vector3 cross(const Vector3& u, const Vector3& v) {
    return Vector3(u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x);
}

Vector3 min(const Vector3& a, const Vector3& b) {
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}

Vector3 max(const Vector3& a, const Vector3& b) {
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

struct AABB {
    alignas(16) Vector3 min;
    alignas(16) Vector3 max;

    AABB(const Vector3& min, const Vector3& max)
        : min(min), max(max) {}

    AABB() = default;

    bool operator==(const AABB& other) const {
        return min == other.min && max == other.max;
    }
};

AABB mergeAABB(const AABB& aabb1, const AABB& aabb2) {
    Vector3 min = Vector3(
        std::min(aabb1.min.x, aabb2.min.x),
        std::min(aabb1.min.y, aabb2.min.y),
        std::min(aabb1.min.z, aabb2.min.z));
    Vector3 max = Vector3(
        std::max(aabb1.max.x, aabb2.max.x),
        std::max(aabb1.max.y, aabb2.max.y),
        std::max(aabb1.max.z, aabb2.max.z));
    return AABB(min, max);
}

struct BVH_Node {
    AABB aabb;
    int left_index;
    int right_index;

    BVH_Node(const AABB& aabb, int left_index, int right_index)
        : aabb(aabb), left_index(left_index), right_index(right_index) {}

    BVH_Node() = default;

    bool operator==(const BVH_Node& other) const {
        return aabb == other.aabb && left_index == other.left_index && right_index == other.right_index;
    }
};

struct Triangle {
    alignas(16) Vector3 v0;
    alignas(16) Vector3 v1;
    alignas(16) Vector3 v2;
    alignas(16) Vector3 normal;
    alignas(16) Vector3 color;
    AABB bounding_box;

    bool operator==(const Triangle& other) const
    {
        return v0 == other.v0 && v1 == other.v1 && v2 == other.v2 && normal == other.normal && bounding_box == other.bounding_box;
    }
};

struct Perfomance {
    clock_t begin;
    clock_t delta;
    double ms;

    void start() {
        begin = clock();
    }

    void stop() {
        delta = clock() - begin;
        ms = (delta / (double)CLOCKS_PER_SEC) * 1000.0;
    }

    Perfomance() = default;
};

inline std::ostream& operator<<(std::ostream& out, const Perfomance& p) {
    return out << "The operation took " << p.ms << " ms.";
}

float degrees_to_radians(float degrees) {
    return degrees * (PI / 180.0f);
}

float radians_to_degrees(float radians) {
    return radians * (180.0f / PI);
}