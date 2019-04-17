#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>

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

    inline Vec2<T> operator +(const Vec2<T> &v) const { return { x+v.x, y+v.y }; }

    inline Vec2<T> operator -(const Vec2<T> &v) const { return { x-v.x, y-v.y }; }

    inline Vec2<T> operator -() const { return { -x, -y }; }

    inline Vec2<T> operator *(int scalar) const { return { scalar*x, scalar*y }; }

    inline Vec2<float> operator *(float scalar) const { return { scalar*x, scalar*y }; }

    inline T operator *(const Vec2<T> &v) const { return x*v.x + y*v.y; }

    inline Vec3<T> operator ^(const Vec2<T> &v) const {
        return { y - v.y, v.x - x, x*v.y - y*v.x };
    }

    inline bool operator ==(const Vec2<T> &v) const { return x==v.x && y==v.y; }

    inline float magnitude() const { return std::sqrt(x*x + y*y); }

    inline Vec2<T> perpendicular() const { return { -y, x }; }

    inline Vec2<float> normalized() const { return (*this) * float(1.0 / magnitude()); }
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

    inline Vec3<T> operator +(const Vec3<T> &v) const { return { x+v.x, y+v.y, z+v.z }; }

    inline Vec3<T> operator -(const Vec3<T> &v) const { return { x-v.x, y-v.y, z-v.z }; }

    inline Vec3<T> operator -() const { return { -x, -y, -z }; }

    inline Vec3<T> operator *(int scalar) const { return { scalar*x, scalar*y, scalar*z }; }

    inline Vec3<float> operator *(float scalar) const { return { scalar*x, scalar*y, scalar*z }; }

    inline T operator *(const Vec3<T> &v) const { return x*v.x + y*v.y + z*v.z; }

    inline Vec3<T> operator ^(const Vec3<T> &v) const {
        return { y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x };
    }

    inline bool operator ==(const Vec2<T> &v) const { return x==v.x && y==v.y && z==v.z; }

    inline float magnitude() const { return std::sqrt(x*x + y*y + z*z); }

    inline Vec3<float> normalized() const { return (*this) * float(1.0 / magnitude()); }
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;

template <typename T>
Vec3f barycentricCoords(Vec2<T> ab, Vec2<T> ac, Vec2<T> ap)
{
    auto dotABAC = ab * ac;
    auto dotABAB = ab * ab;
    auto dotACAC = ac * ac;
    auto dotAPAB = ap * ab;
    auto dotAPAC = ap * ac;
    auto denom = dotACAC*dotABAB - dotABAC*dotABAC;
    float inverseDenom = 1.0f / denom;
    auto uNumer = dotABAB*dotAPAC - dotABAC*dotAPAB;
    float u = uNumer * inverseDenom;
    auto vNumer = dotACAC*dotAPAB - dotABAC*dotAPAC;
    float v = vNumer * inverseDenom;
    float w = float(denom - uNumer - vNumer);

    return Vec3f(u,v,w);
}

#endif // __GEOMETRY_H__
