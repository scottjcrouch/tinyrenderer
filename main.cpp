#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor black = TGAColor(  0,   0,   0, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(  0, 255,   0, 255);
const TGAColor blue  = TGAColor(  0,   0, 255, 255);

auto model(std::make_unique<Model>("obj/african_head.obj"));

void drawLine(Vec2i p0, Vec2i p1,
              TGAImage &image, const TGAColor &color)
{
    // To draw a line, the basic method is to draw one dot for every x
    // coordinate between the two points, and interpolate on y.  However, if
    // the line is steep on y, then this will result in gaps in the line.  We
    // can solve this problem by checking if the line is steep, and if so,
    // draw by iterating over y while interpolating on x.
    bool isSteep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        isSteep = true;
    }

    // Make p0 be the leftmost point.
    if (p0.x > p1.x) {
        std::swap(p0.x, p1.x);
        std::swap(p0.y, p1.y);
    }

    // Normally, each iteration we increment x by 1 and y by dy/dx, then round
    // y to the nearest pixel.  To save division ops, we pull the calculation
    // of dy/dx out of the loop (derror).  From there, using dimensional
    // analysis to eliminate terms, we can eliminate the need to use division
    // ops altogether.
    int dx = p1.x - p0.x;
    int dy = p1.y - p0.y;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    for (int x = p0.x, y = p0.y; x <= p1.x; x++) {
        if (isSteep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }

        error2 += derror2;
        if (error2 > dx) {
            y += ((p1.y > p0.y) ? 1 : -1);
            error2 -= (dx * 2);
        }
    }
}

void drawTriangle(const Vec2i &a, const Vec2i &b, const Vec2i &c,
                  TGAImage &image, const TGAColor &color)
{
    drawLine(a, b, image, color);
    drawLine(b, c, image, color);
    drawLine(c, a, image, color);
}

void drawSquare(const Vec2i &min, const Vec2i &max,
                TGAImage &image, const TGAColor &color)
{
    drawLine(min, {min.x, max.y}, image, color);
    drawLine(min, {max.x, min.y}, image, color);
    drawLine(max, {min.x, max.y}, image, color);
    drawLine(max, {max.x, min.y}, image, color);
}

void fillTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c,
                  std::vector<float> &zBuffer, TGAImage &image, TGAColor color)
{
    Vec3f ab(b - a);
    Vec3f ac(c - a);

    // If the triangle's vertices are collinear, then don't draw anything.
    if ((ab ^ ac).z == 0.0f) {
        return;
    }

    // Get the on-screen bounding box for the triangle.
    int width = image.get_width();
    int height = image.get_height();
    Vec2i imageMin = { 0, 0 };
    Vec2i imageMax = { width - 1, height - 1 };
    Vec2i lowBound(int(std::min({ a.x, b.x, c.x })),
                   int(std::min({ a.y, b.y, c.y })));
    Vec2i highBound(int(std::max({ a.x, b.x, c.x })),
                    int(std::max({ a.y, b.y, c.y })));
    clampVec2(lowBound, imageMin, imageMax);
    clampVec2(lowBound, imageMin, imageMax);

    Vec3f p;
    for (p.y = lowBound.y; p.y <= highBound.y; p.y++) {
        for (p.x = lowBound.x; p.x <= highBound.x; p.x++) {
            Vec3f ap(p - a);
            Vec3f bCoords = barycentricCoords(ab, ac, ap);
            if (bCoords.u < 0 ||
                bCoords.v < 0 ||
                bCoords.w < 0) {
                continue;
            }
            p.z = a.z*bCoords.w + b.z*bCoords.u + c.z*bCoords.v;
            if (zBuffer[int(p.y*width + p.x)] >= p.z) {
                continue;
            }
            zBuffer[int(p.y*width + p.x)] = p.z;
            image.set(p.x, p.y, color);
        }
    }
}

void lesson3()
{
    constexpr int width  = 800;
    constexpr int height = 800;
    TGAImage image(width, height, TGAImage::RGB);
    std::vector<float> zBuffer(width * height, std::numeric_limits<float>::lowest());

    for (int i = 0; i < model->numFaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f worldCoords[3];
        Vec3f screenCoords[3];
        for (int j = 0; j < 3; j++) {
            worldCoords[j] = model->vert(face[j]);
            screenCoords[j] =
                Vec3f((worldCoords[j].x + 1.0) * width / 2.0,
                      (worldCoords[j].y + 1.0) * height / 2.0,
                      worldCoords[j].z);
        }

        Vec3f lightVec(0, 0, -1);
        Vec3f ab(worldCoords[1] - worldCoords[0]);
        Vec3f ac(worldCoords[2] - worldCoords[0]);
        Vec3f faceNormal = (ac ^ ab).normalized();
        float intensity = (faceNormal * lightVec) * 255.0;
        if (intensity > 0) { // Cull backfaces
            TGAColor faceIllum = TGAColor(intensity, intensity, intensity, 255);
            fillTriangle(screenCoords[0], screenCoords[1], screenCoords[2],
                         zBuffer, image, faceIllum);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
}

int main(int argc, char** argv)
{
    lesson3();
    return 0;
}
