#include <vector>
#include <algorithm>

#include "tgaimage.h"
#include "geometry.h"
#include "gl.h"

Matrix4x4 viewport;
Matrix4x4 projection;
Matrix4x4 modelview;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor black = TGAColor(  0,   0,   0, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(  0, 255,   0, 255);
const TGAColor blue  = TGAColor(  0,   0, 255, 255);

void view(int minX, int minY, int width, int height)
{
    constexpr float depth = 255.0;
    viewport[0][0] = width/2;
    viewport[1][1] = height/2;
    viewport[2][2] = depth/2;
    viewport[0][3] = minX + width/2;
    viewport[1][3] = minY + height/2;
    viewport[2][3] = depth/2;
}

void project(const float coeff)
{
    projection[3][2] = coeff;
}

static Matrix4x4 translate(const float xOffset, const float yOffset, const float zOffset)
{
    Matrix4x4 result;
    result[0][3] = xOffset;
    result[1][3] = yOffset;
    result[2][3] = zOffset;
    return result;
}

static Matrix4x4 scale(const float xFactor, const float yFactor, const float zFactor)
{
    Matrix4x4 result;
    result[0][0] = xFactor;
    result[1][1] = yFactor;
    result[2][2] = zFactor;
    return result;
}

static Matrix4x4 basis(const Vec3f &col0, const Vec3f &col1, const Vec3f &col2)
{
    Matrix4x4 result;
    for (int i = 0; i < 3; i++) {
        result[0][i] = col0.raw[i];
        result[1][i] = col1.raw[i];
        result[2][i] = col2.raw[i];
    }
    return result;
}

void lookAt(Vec3f eye, Vec3f point, Vec3f up)
{
    Vec3f zPrime = (eye - point).normalized();
    assert((up ^ zPrime) != Vec3f(0,0,0)); // up and gaze direction can't be parallel
    Vec3f xPrime = (up ^ zPrime).normalized();
    Vec3f yPrime = (zPrime ^ xPrime).normalized();
    Matrix4x4 translatePointToOrigin = translate(-point.x, -point.y, -point.z);
    Matrix4x4 inverseAxesTransform = basis(xPrime, yPrime, zPrime);
    modelview = inverseAxesTransform * translatePointToOrigin;
}

void drawTriangle(const std::array<Vec3f, 3> &vertices,
                  IShader &shader,
                  TGAImage &image,
                  std::vector<float> &zBuffer)
{
    const Vec3f &a = vertices[0];
    const Vec3f &b = vertices[1];
    const Vec3f &c = vertices[2];

    Vec3f ab(b - a);
    Vec3f ac(c - a);

    // Backface culling.
    if ((ab ^ ac).z <= 0.0f) {
        return;
    }

    // Get the bounding box of the triangle.
    int width = image.get_width();
    int height = image.get_height();
    Vec2i imageMin = { 0, 0 };
    Vec2i imageMax = { width, height };
    Vec2i lowBound(int(std::min({ a.x, b.x, c.x })),
                   int(std::min({ a.y, b.y, c.y })));
    Vec2i highBound(int(std::ceil(std::max({ a.x, b.x, c.x }))),
                    int(std::ceil(std::max({ a.y, b.y, c.y }))));
    clampVec2(lowBound, imageMin, imageMax);
    clampVec2(highBound, imageMin, imageMax);

    Vec3f p;
    for (p.y = lowBound.y; p.y < highBound.y; p.y++) {
        for (p.x = lowBound.x; p.x < highBound.x; p.x++) {
            Vec3f ap(p - a);
            Vec3f bary = barycentricCoords(ab, ac, ap);
            if (bary.u < 0 ||
                bary.v < 0 ||
                bary.w < 0) {
                continue;
            }
            p.z = a.z*bary.u + b.z*bary.v + c.z*bary.w;
            if (zBuffer[int(p.y*width + p.x)] >= p.z) {
                continue;
            }
            zBuffer[int(p.y*width + p.x)] = p.z;
            TGAColor color;
            bool discard = shader.fragment(bary, color);
            if (!discard) {
                image.set(p.x, p.y, color);
            }
        }
    }
}
