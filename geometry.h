#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <cassert>
#include <array>

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
    inline bool operator !=(const Vec2<T> &v) const { return !(*this == v); }
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

    inline bool operator ==(const Vec3<T> &v) const { return x==v.x && y==v.y && z==v.z; }
    inline bool operator !=(const Vec3<T> &v) const { return !(*this == v); }
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

template <class T>
void clamp(T &x, const T &low, const T &high)
{
    if (x < low) x = low;
    if (x > high) x = high;
}

template <class T>
void clampVec2(Vec2<T> &v, const Vec2<T> &low, const Vec2<T> &high)
{
    clamp(v.x, low.x, high.x);
    clamp(v.y, low.y, high.y);
}

template <class T>
void clampVec3(Vec3<T> &v, const Vec3<T> &low, const Vec3<T> &high)
{
    clamp(v.x, low.x, high.x);
    clamp(v.y, low.y, high.y);
    clamp(v.z, low.z, high.z);
}

struct Matrix
{
    std::array<std::array<float, 4>, 4> m;

    Matrix() : m{{{1,0,0,0},
                  {0,1,0,0},
                  {0,0,1,0},
                  {0,0,0,1}}} { }

    inline std::array<float, 4>& operator [] (const int row) { return m[row]; }

    inline Vec3f operator *(const Vec3f &v) const {
        Vec3f result = { v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2] + m[0][3],
                         v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2] + m[1][3],
                         v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2] + m[2][3] };
        return result * (1.0f / (v.x*m[3][0] + v.y*m[3][1] + v.z*m[3][2] + m[3][3]));
    }

    inline Matrix operator *(const Matrix &rhs) const {
        Matrix result;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                result.m[row][col] = 0.0f;
                for (int i = 0; i < 4; i++) {
                    result.m[row][col] += m[row][i] * rhs.m[i][col];
                }
            }
        }
        return result;
    }
};

#endif // __GEOMETRY_H__
