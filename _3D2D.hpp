#pragma once

#include <math.h>
#define PI 3.14159265368979f

struct Angle
{
    float pitch, yaw;

    Angle(float x, float y)
    {
        pitch = x;
        yaw   = y;
    }

    void normalize(void)
    {
        if (yaw >  180.0f) yaw -= 360.0f;
        if (yaw < -180.0f) yaw += 360.0f;
        
        if (pitch >  89.0f) pitch -= 180.0f;
        if (pitch < -89.0f) pitch += 180.0f;
    }

    Angle operator * (float scale)
    {
        return Angle (
            pitch * scale,
            yaw   * scale
        );
    }

    void __cout(void)
    { std::cout << "(" << pitch << ", " << yaw << ")" << std::endl;
    }
};

struct Vector2
{
    float x, y;

    Vector2(float x, float y)
    {
        this->x = x;
        this->y = y;
    }

    Vector2 operator - (Vector2 other)
    {
        return Vector2 (
            x - other.x,
            y - other.y
        );
    }

    Vector2 operator + (Vector2 other)
    {
        return Vector2 (
            x + other.x,
            y + other.y
        );
    }

    Vector2 operator * (float scale)
    {
        return Vector2 (
            x * scale,
            y * scale
        );
    }

    void __cout(void)
    { std::cout << "(" << x << ", " << y << ")" << std::endl;
    }
};

struct Vector3
{
    float x, y, z;

    Vector3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    float distanceTo(Vector3 vector)
    {
        Vector3 delta = vector - (*this);

        return sqrt((delta.x * delta.x) + (delta.y * delta.y) + (delta.z * delta.z));
    }

    Vector3 operator - (Vector3 other)
    {
        return Vector3 (
            x - other.x,
            y - other.y,
            z - other.z
        );
    }

    Vector3 operator + (Vector3 other)
    {
        return Vector3 (
            x + other.x,
            y + other.y,
            z + other.z
        );
    }

    Vector3 operator * (float scale)
    {
        return Vector3 (
            x * scale,
            y * scale,
            z * scale
        );
    }

    void __cout(void)
    { std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};

struct Vector4
{
    float x, y, z, w;

    Vector4(float x, float y, float z, float w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
};