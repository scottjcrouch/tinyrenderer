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
    T pa = -ap;
    auto cross = Vec3f(ac.x, ab.x, pa.x) ^ Vec3f(ac.y, ab.y, pa.y);
    assert(cross.z != 0.0f);
    return Vec3f(1.0 - ((cross.x + cross.y) / cross.z),
                 cross.y / cross.z,
                 cross.x / cross.z);
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

struct Matrix4x4;
struct Matrix3x3;
struct Matrix2x3;

struct Matrix4x4
{
    std::array<std::array<float, 4>, 4> m;

    Matrix4x4() : m{{{1,0,0,0},
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

    inline Matrix4x4 operator *(const Matrix4x4 &rhs) const {
        Matrix4x4 result;
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

    inline Matrix4x4 operator *(const float &scalar) const {
        Matrix4x4 result;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                result.m[row][col] = m[row][col] * scalar;
            }
        }
        return result;
    }

    inline float determinant() {
        return
            m[0][3]*m[1][2]*m[2][1]*m[3][0] - m[0][2]*m[1][3]*m[2][1]*m[3][0] -
            m[0][3]*m[1][1]*m[2][2]*m[3][0] + m[0][1]*m[1][3]*m[2][2]*m[3][0] +
            m[0][2]*m[1][1]*m[2][3]*m[3][0] - m[0][1]*m[1][2]*m[2][3]*m[3][0] -
            m[0][3]*m[1][2]*m[2][0]*m[3][1] + m[0][2]*m[1][3]*m[2][0]*m[3][1] +
            m[0][3]*m[1][0]*m[2][2]*m[3][1] - m[0][0]*m[1][3]*m[2][2]*m[3][1] -
            m[0][2]*m[1][0]*m[2][3]*m[3][1] + m[0][0]*m[1][2]*m[2][3]*m[3][1] +
            m[0][3]*m[1][1]*m[2][0]*m[3][2] - m[0][1]*m[1][3]*m[2][0]*m[3][2] -
            m[0][3]*m[1][0]*m[2][1]*m[3][2] + m[0][0]*m[1][3]*m[2][1]*m[3][2] +
            m[0][1]*m[1][0]*m[2][3]*m[3][2] - m[0][0]*m[1][1]*m[2][3]*m[3][2] -
            m[0][2]*m[1][1]*m[2][0]*m[3][3] + m[0][1]*m[1][2]*m[2][0]*m[3][3] +
            m[0][2]*m[1][0]*m[2][1]*m[3][3] - m[0][0]*m[1][2]*m[2][1]*m[3][3] -
            m[0][1]*m[1][0]*m[2][2]*m[3][3] + m[0][0]*m[1][1]*m[2][2]*m[3][3];
    }

    inline Matrix4x4 minor(int row, int col) {
        Matrix4x4 result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result[i][j] = m[i<row ? i : i+1][j<col ? j : j+1];
            }
        }
        return result;
    }

    inline float cofactor(int row, int col) {
        Matrix4x4 min = minor(row, col);

        double aei = min[0][0]*min[1][1]*min[2][2];
	double afh = min[0][0]*min[1][2]*min[2][1];
	double bfg = min[0][1]*min[1][2]*min[2][0];
	double bdi = min[0][1]*min[1][0]*min[2][2];
	double cdh = min[0][2]*min[1][0]*min[2][1];
	double ceg = min[0][2]*min[1][1]*min[2][0];
	float det = (aei - afh) + (bfg - bdi) + (cdh - ceg);

        return det * ((row + col) % 2 ? -1 : 1);
    }

    inline Matrix4x4 adjugate() {
        Matrix4x4 result;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                result[row][col] = cofactor(row, col);
            }
        }
        return result;
    }

    inline Matrix4x4 inverse() {
        float det = determinant();
        assert(det != 0.0f);
        return adjugate() * (1 / det);
    }
};

struct Matrix3x3
{
    std::array<std::array<float, 3>, 3> m;

    Matrix3x3() : m{{{1,0,0},
                     {0,1,0},
                     {0,0,1}}} { }

    Matrix3x3(Vec3f col0, Vec3f col1, Vec3f col2) : m{
        {{col0.x, col1.x, col2.x},
         {col0.y, col1.y, col2.y},
         {col0.z, col1.z, col2.z}}
    } { }

    inline void setCol(Vec3f v, int columnIndex) {
        assert(columnIndex >= 0 && columnIndex <= 3);
        m[0][columnIndex] = v.x;
        m[1][columnIndex] = v.y;
        m[2][columnIndex] = v.z;
    }

    inline std::array<float, 3>& operator [] (const int row) { return m[row]; }

    inline Vec3f operator *(const Vec3f &v) const {
        Vec3f result = { v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2],
                         v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2],
                         v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2] };
        return result;
    }

    inline Matrix3x3 operator *(const Matrix3x3 &rhs) const {
        Matrix3x3 result;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                result.m[row][col] = 0.0f;
                for (int i = 0; i < 3; i++) {
                    result.m[row][col] += m[row][i] * rhs.m[i][col];
                }
            }
        }
        return result;
    }

    inline Matrix3x3 operator *(const float &scalar) const {
        Matrix3x3 result;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                result.m[row][col] = m[row][col] * scalar;
            }
        }
        return result;
    }
};

struct Matrix2x3
{
    std::array<std::array<float, 3>, 2> m;

    Matrix2x3() : m{0} { }

    Matrix2x3(Vec2f col0, Vec2f col1, Vec2f col2) : m{
        { { col0.x, col1.x, col2.x },
          { col0.y, col1.y, col2.y } }
    } { }

    inline void setCol(Vec2f v, int columnIndex) {
        assert(columnIndex >= 0 && columnIndex <= 3);
        m[0][columnIndex] = v.x;
        m[1][columnIndex] = v.y;
    }

    inline std::array<float, 3>& operator [] (const int row) { return m[row]; }

    inline Vec2f operator *(const Vec3f &v) const {
        Vec2f result = { v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2],
                         v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2] };
        return result;
    }

    inline Matrix2x3 operator *(const float &scalar) const {
        Matrix2x3 result;
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 3; col++) {
                result.m[row][col] = m[row][col] * scalar;
            }
        }
        return result;
    }
};

#endif // __GEOMETRY_H__
