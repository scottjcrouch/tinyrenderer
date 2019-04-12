#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>

template <class T>
struct Vec2
{
    union {
        struct { T x, y; };
        struct { T u, v; };
        T raw[2];
    };
    Vec2() : x(0), y(0) { }
    Vec2(T _x, T _y) : x(_x), y(_y) { }
    inline Vec2<T> operator +(const Vec2<T> &v) const { return Vec2<T>(x+v.x, y+v.y); }
    inline Vec2<T> operator -(const Vec2<T> &v) const { return Vec2<T>(x-v.x, y-v.y); }
    inline bool operator ==(const Vec2<T> &v) const { return x==v.x && y==v.y; }
};

template <class T>
struct Vec3
{
    union {
        struct { T x, y, z; };
        struct { T u, v, w; };
        T raw[3];
    };
    Vec3() : x(0), y(0), z(0) { }
    Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) { }
    inline Vec3<T> operator +(const Vec3<T> &v) const { return Vec3<T>(x+v.x, y+v.y, z+v.z); }
    inline Vec3<T> operator -(const Vec3<T> &v) const { return Vec3<T>(x-v.x, y-v.y, z-v.z); }
    inline bool operator ==(const Vec2<T> &v) const { return x==v.x && y==v.y && z==v.z; }
};

template <class T>
float mag(const Vec2<T> &a)
{
    return std::sqrt(a.x*a.x + a.y*a.y);
}

template <class T>
float mag(const Vec3<T> &a)
{
    return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

template <class T>
Vec2<T> perp(const Vec2<T> &a)
{
    return Vec2<T>(-a.y, a.x);
}

template <class T>
T dotProd(Vec2<T> a, Vec2<T> b)
{
    return a.x*b.x + a.y*b.y;
}

template <class T>
Vec3<T> crossProd(Vec2<T> a, Vec2<T> b)
{
    return Vec3<T>(a.y - b.y,
                   b.x - a.x,
                   a.x*b.y - a.y*b.x);
}

template <class T>
Vec3<T> crossProd(Vec3<T> a, Vec3<T> b)
{
    return Vec3<T>(a.y*b.z - a.z*b.y,
                   a.z*b.x - a.x*b.z,
                   a.x*b.y - a.y*b.x);
}

template <class T>
Vec2<float> barycentric(Vec2<T> ab, Vec2<T> ac, Vec2<T> ap)
{
    auto dotABAC = dotProd(ab,ac);
    auto dotABAB = dotProd(ab,ab);
    auto dotACAC = dotProd(ac,ac);
    auto dotAPAB = dotProd(ap,ab);
    auto dotAPAC = dotProd(ap,ac);
    float invDenom = 1.0 / (dotACAC*dotABAB - dotABAC*dotABAC);
    float u = ((dotABAB * dotAPAC) - (dotABAC * dotAPAB)) * invDenom;
    float v = ((dotACAC * dotAPAB) - (dotABAC * dotAPAC)) * invDenom;

    return Vec2<float>(u,v);
}

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;

#endif // __GEOMETRY_H__
