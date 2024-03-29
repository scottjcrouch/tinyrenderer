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

    inline T& operator [] (int row) { return raw[row]; }
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

    inline T& operator [] (int row) { return raw[row]; }
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

template <typename T>
struct Vec4
{
    union {
        struct { T a, b, c, d; };
        T raw[4];
    };

    Vec4() : a(0), b(0), c(0), d(0) { }
    Vec4(T _a, T _b, T _c, T _d) : a(_a), b(_b), c(_c), d(_d) { }
    Vec4(Vec3<T> v, int _d) : a(v.x), b(v.y), c(v.z), d(_d) { }

    inline T& operator [] (int row) { return raw[row]; }

    inline bool operator ==(const Vec4<T> &v) const {
        return a==v.a && b==v.b && c==v.c && d==v.d;
    }
    inline bool operator !=(const Vec4<T> &v) const {
        return !(*this == v);
    }
    inline Vec4<T> operator +(const Vec4<T> &v) const {
        return { a+v.a, b+v.b, c+v.c, d+v.d };
    }
    inline Vec4<T> operator -(const Vec4<T> &v) const {
        return { a-v.a, b-v.b, c-v.c, d-v.d };
    }
    inline Vec4<T> operator -() const {
        return { -a, -b, -c, -d };
    }
    inline Vec4<T> operator *(int scalar) const {
        return { scalar*a, scalar*b, scalar*c, scalar*d };
    }
    inline Vec4<float> operator *(float scalar) const {
        return { scalar*a, scalar*b, scalar*c, scalar*d };
    }
    inline T operator *(const Vec4<T> &v) const {
        return a*v.a + b*v.b + c*v.c + d*v.d;
    }

    float magnitude() const { return std::sqrt(a*a + b*b + c*c + d*d); }
    Vec4<float> normalized() const { return (*this) * float(1.0 / magnitude()); }
    Vec3<float> homogenized() const {
        return Vec3<float>(a/d, b/d, c/d);
    }
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;
using Vec4f = Vec4<float>;
using Vec4i = Vec4<int>;

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
struct Matrix2x2;

struct Matrix2x2
{
    std::array<std::array<float, 2>, 2> m;

    static Matrix2x2 identity() {
        Matrix2x2 result;
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
                result[row][col] = float(row == col);
            }
        }
        return result;
    }

    inline std::array<float, 2>& operator [] (int row) { return m[row]; }
};

struct Matrix2x3
{
    std::array<std::array<float, 3>, 2> m;

    Matrix2x3() : m{0} { }

    inline void setCol(int columnIndex, const Vec2f& v) {
        assert(columnIndex >= 0 && columnIndex <= 3);
        m[0][columnIndex] = v.x;
        m[1][columnIndex] = v.y;
    }

    inline void setRow(int rowIndex, const Vec3f& v) {
        assert(rowIndex >= 0 && rowIndex <= 2);
        m[rowIndex][0] = v.x;
        m[rowIndex][1] = v.y;
        m[rowIndex][1] = v.z;
    }

    inline Vec2f getCol(int columnIndex) {
        assert(columnIndex >= 0 && columnIndex <= 3);
        return Vec2f(m[0][columnIndex], m[1][columnIndex]);
    }

    inline std::array<float, 3>& operator [] (int row) { return m[row]; }

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

struct Matrix3x3
{
    std::array<std::array<float, 3>, 3> m;

    static Matrix3x3 identity() {
        Matrix3x3 result;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                result[row][col] = float(row == col);
            }
        }
        return result;
    }

    inline void setCol(int columnIndex, const Vec3f& v) {
        assert(columnIndex >= 0 && columnIndex <= 3);
        m[0][columnIndex] = v.x;
        m[1][columnIndex] = v.y;
        m[2][columnIndex] = v.z;
    }

    inline void setRow(int rowIndex, const Vec3f& v) {
        assert(rowIndex >= 0 && rowIndex <= 3);
        m[rowIndex][0] = v.x;
        m[rowIndex][1] = v.y;
        m[rowIndex][2] = v.z;
    }

    inline Vec3f getCol(int columnIndex) {
        assert(columnIndex >= 0 && columnIndex <= 3);
        return Vec3f(m[0][columnIndex], m[1][columnIndex], m[2][columnIndex]);
    }

    inline std::array<float, 3>& operator [] (int row) { return m[row]; }

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
                result[row][col] = m[row][col] * scalar;
            }
        }
        return result;
    }

    inline float determinant() {
        return
            m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
            m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
            m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }

    inline Matrix2x2 minor(int row, int col) {
        Matrix2x2 result;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                result[i][j] = m[i<row ? i : i+1][j<col ? j : j+1];
            }
        }
        return result;
    }

    inline float cofactor(int row, int col) {
        Matrix2x2 min = minor(row, col);

        float ad = min[0][0]*min[1][1];
        float bc = min[0][1]*min[1][0];
        float det = ad-bc;

        return det * ((row + col) % 2 ? -1 : 1);
    }

    inline Matrix3x3 adjugate() {
        Matrix3x3 result;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                result[row][col] = cofactor(row, col);
            }
        }
        return result;
    }

    inline Matrix3x3 inverseTranspose() {
        float det = determinant();
        assert(det != 0.0f);
        return adjugate() * (1 / det);
    }

    inline Matrix3x3 transpose() {
        Matrix3x3 result;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                result[row][col] = m[col][row];
            }
        }
        return result;
    }

    inline Matrix3x3 inverse() {
        return inverseTranspose().transpose();
    }
};

struct Matrix4x4
{
    std::array<std::array<float, 4>, 4> m;

    static Matrix4x4 identity() {
        Matrix4x4 result;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                result[row][col] = float(row == col);
            }
        }
        return result;
    }

    inline std::array<float, 4>& operator [] (int row) { return m[row]; }

    inline Vec3f operator *(const Vec3f &v) const {
        Vec3f result = { v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2] + m[0][3],
                         v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2] + m[1][3],
                         v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2] + m[2][3] };
        return result * (1.0f / (v.x*m[3][0] + v.y*m[3][1] + v.z*m[3][2] + m[3][3]));
    }

    inline Vec4f operator *(const Vec4f &v) const {
        Vec4f result = { v.a*m[0][0] + v.b*m[0][1] + v.c*m[0][2] + v.d*m[0][3],
                         v.a*m[1][0] + v.b*m[1][1] + v.c*m[1][2] + v.d*m[1][3],
                         v.a*m[2][0] + v.b*m[2][1] + v.c*m[2][2] + v.d*m[2][3],
                         v.a*m[3][0] + v.b*m[3][1] + v.c*m[3][2] + v.d*m[3][3]};
        return result;
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
                result[row][col] = m[row][col] * scalar;
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

    inline Matrix3x3 minor(int row, int col) {
        Matrix3x3 result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result[i][j] = m[i<row ? i : i+1][j<col ? j : j+1];
            }
        }
        return result;
    }

    inline float cofactor(int row, int col) {
        Matrix3x3 min = minor(row, col);

        float aei = min[0][0]*min[1][1]*min[2][2];
	float afh = min[0][0]*min[1][2]*min[2][1];
	float bfg = min[0][1]*min[1][2]*min[2][0];
	float bdi = min[0][1]*min[1][0]*min[2][2];
	float cdh = min[0][2]*min[1][0]*min[2][1];
	float ceg = min[0][2]*min[1][1]*min[2][0];
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

    inline Matrix4x4 inverseTranspose() {
        float det = determinant();
        assert(det != 0.0f);
        return adjugate() * (1 / det);
    }

    inline Matrix4x4 transpose() {
        Matrix4x4 result;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                result[row][col] = m[col][row];
            }
        }
        return result;
    }

    inline Matrix4x4 inverse() {
        return inverseTranspose().transpose();
    }
};

#endif // __GEOMETRY_H__
