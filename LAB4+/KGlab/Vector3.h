// Гаврилов А.Г.
// 27.12.2024
#pragma once

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <utility>

class Vector3
{
    double* coords;

public:
    double x() const { return coords[0]; }
    double y() const { return coords[1]; }
    double z() const { return coords[2]; }

    // Конструкторы
#pragma region Constructors
    Vector3() { coords = new double[3] {0, 0, 0}; }

    Vector3(double x, double y, double z) : Vector3() {
        coords[0] = x; coords[1] = y; coords[2] = z;
    }

    Vector3(std::initializer_list<double>& list) {
        coords[0] = *(list.begin());
        coords[1] = *(list.begin() + 1);
        coords[2] = *(list.begin() + 2);
    }

    Vector3(const Vector3& vec) {
        coords = new double[3];
        std::memcpy(coords, vec.coords, 3 * sizeof(double));
    }

    Vector3(Vector3&& vec) noexcept {
        coords = vec.coords;
        vec.coords = nullptr;
    }
#pragma endregion

    ~Vector3() { delete[] coords; }

#pragma region Operators
    void setCoords(double x, double y, double z) {
        coords[0] = x; coords[1] = y; coords[2] = z;
    }

    Vector3 operator+(const Vector3& vec) const {
        return Vector3(coords[0] + vec.coords[0],
            coords[1] + vec.coords[1],
            coords[2] + vec.coords[2]);
    }

    Vector3 operator-(const Vector3& vec) const {
        return Vector3(coords[0] - vec.coords[0],
            coords[1] - vec.coords[1],
            coords[2] - vec.coords[2]);
    }

    Vector3 operator-() const { return Vector3(-coords[0], -coords[1], -coords[2]); }
    Vector3 operator+() const { return *this; }

    template <typename T> Vector3 operator*(const T k) const {
        return Vector3(coords[0] * k, coords[1] * k, coords[2] * k);
    }

    template <typename T> Vector3 operator/(const T k) const {
        return Vector3(coords[0] / k, coords[1] / k, coords[2] / k);
    }

    Vector3& operator=(const Vector3& vec) {
        std::memcpy(coords, vec.coords, 3 * sizeof(double));
        return *this;
    }

    Vector3& operator=(Vector3&& vec) {
        if (this != &vec) {
            delete[] coords;
            coords = vec.coords;
            vec.coords = nullptr;
        }
        return *this;
    }

    double length() const {
        return sqrt(coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2]);
    }

    // ИСПРАВЛЕННЫЙ МЕТОД: теперь const
    Vector3 normalize() const {
        double l = length();
        if (l < 1e-12) return Vector3(0, 0, 0);
        return Vector3(coords[0] / l, coords[1] / l, coords[2] / l);
    }

    Vector3 operator^(const Vector3& v) const {
        return Vector3(coords[1] * v.coords[2] - coords[2] * v.coords[1],
            coords[2] * v.coords[0] - coords[0] * v.coords[2],
            coords[0] * v.coords[1] - coords[1] * v.coords[0]);
    }

    double operator&(const Vector3& v) const {
        return coords[0] * v.coords[0] + coords[1] * v.coords[1] + coords[2] * v.coords[2];
    }

    const double* operator()() const { return coords; }

    static Vector3 Z() { return Vector3(0, 0, 1); }
    static Vector3 X() { return Vector3(0, 1, 0); }
    static Vector3 Y() { return Vector3(1, 0, 0); }
#pragma endregion
};

template <typename T> Vector3 operator*(T arg, const Vector3& v) {
    return Vector3(v.x() * arg, v.y() * arg, v.z() * arg);
}
template <typename T> Vector3 operator/(T arg, const Vector3& v) {
    return Vector3(v.x() / arg, v.y() / arg, v.z() / arg);
}