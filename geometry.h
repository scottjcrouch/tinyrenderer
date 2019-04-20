#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <cassert>

template <typename T> struct Vec2;
template <typename T> struct Vec3;

template <typename T>
struct Vec2
{
    union {
        struct { T x, y; };
        struct { T u, v; };
        T raw[2];
    };

    Vec2() : x(0), y(0) { }
    Vec2(T _x, T _y) : x(_x), y(_y) { }

    inline bool operator ==(const Vec2<T> &v) const { return x==v.x && y==v.y; }
    inline Vec2<T> operator +(const Vec2<T> &v) const { return { x+v.x, y+v.y }; }
    inline Vec2<T> operator -(const Vec2<T> &v) const { return { x-v.x, y-v.y }; }
    inline Vec2<T> operator -() const { return { -x, -y }; }
    inline Vec2<T> operator *(int scalar) const { return { scalar*x, scalar*y }; }
    inline Vec2<float> operator *(float scalar) const { return { scalar*x, scalar*y }; }
    inline T operator *(const Vec2<T> &v) const { return x*v.x + y*v.y; }
    inline Vec3<T> operator ^(const Vec2<T> &v) const {
        return { y - v.y, v.x - x, x*v.y - y*v.x };
    }

    float magnitude() const { return std::sqrt(x*x + y*y); }
    Vec2<T> perpendicular() const { return { -y, x }; }
    Vec2<float> normalized() const { return (*this) * float(1.0 / magnitude()); }
};

template <typename T>
struct Vec3
{
    union {
        struct { T x, y, z; };
        struct { T u, v, w; };
        T raw[3];
    };

    Vec3() : x(0), y(0), z(0) { }
    Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) { }

    inline bool operator ==(const Vec2<T> &v) const { return x==v.x && y==v.y && z==v.z; }
    inline Vec3<T> operator +(const Vec3<T> &v) const { return { x+v.x, y+v.y, z+v.z }; }
    inline Vec3<T> operator -(const Vec3<T> &v) const { return { x-v.x, y-v.y, z-v.z }; }
    inline Vec3<T> operator -() const { return { -x, -y, -z }; }
    inline Vec3<T> operator *(int scalar) const { return { scalar*x, scalar*y, scalar*z }; }
    inline Vec3<float> operator *(float scalar) const { return { scalar*x, scalar*y, scalar*z }; }
    inline T operator *(const Vec3<T> &v) const { return x*v.x + y*v.y + z*v.z; }
    inline Vec3<T> operator ^(const Vec3<T> &v) const {
        return { y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x };
    }

    float magnitude() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3<float> normalized() const { return (*this) * float(1.0 / magnitude()); }
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;

template <typename T>
Vec3f barycentricCoords(T ab, T ac, T ap)
{
    T pa(-ap);
    auto cross = Vec3f(ab.x, ac.x, pa.x) ^ Vec3f(ab.y, ac.y, pa.y);
    assert(cross.z != 0.0f);
    return Vec3f(cross.x / cross.z,
                 cross.y / cross.z,
                 1.0 - ((cross.x + cross.y) / cross.z));
}

#endif // __GEOMETRY_H__
