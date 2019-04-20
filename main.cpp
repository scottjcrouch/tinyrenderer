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

void lesson0()
{
    TGAImage image(100, 100, TGAImage::RGB);
    image.set(50, 50, red);
    image.set(51, 51, red);
    image.set(50, 51, red);
    image.set(51, 50, red);
    image.flip_vertically(); // Sets origin at the bottom-left corner of the image
    image.write_tga_file("output.tga");
}

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

void lesson1()
{
    constexpr int width  = 800;
    constexpr int height = 800;
    TGAImage image(width, height, TGAImage::RGB);

    // For each face, draw lines connecting all three points of the triangle.
    // The vertices are defined as floats in the range [-1.0, 1.0].  We obtain
    // screen coords from them by scaling to the height and width of the tga
    // image.
    for (int i = 0; i < model->numFaces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; j++) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j + 1) % 3]);
            Vec2i p0{int((v0.x + 1.0) * width / 2.0),
                    int((v0.y + 1.0) * height / 2.0)};
            Vec2i p1{int((v1.x + 1.0) * width / 2.0),
                    int((v1.y + 1.0) * height / 2.0)};
            drawLine(p0, p1, image, white);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
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

struct TriangleBoundingBoxes
{
    Vec2i lowerMin, lowerMax, upperMin, upperMax;
};

void clamp(Vec2i &v, const Vec2i &lowerBound, const Vec2i &upperBound)
{
    if (v.x < lowerBound.x) v.x = lowerBound.x;
    if (v.x > upperBound.x) v.x = upperBound.x;
    if (v.y < lowerBound.y) v.y = lowerBound.y;
    if (v.y > upperBound.y) v.y = upperBound.y;
}

/*
 * Calculates the upper and lower bounding boxes for a 2D triangle, the sum
 * area of which being smaller than a single bounding box.
 */
TriangleBoundingBoxes getTriangleBoundingBoxes(const Vec2i &a,
                                               const Vec2i &b,
                                               const Vec2i &c,
                                               const Vec2i &lowBound,
                                               const Vec2i &highBound)
{
    // Sort the triangle vertices by y coord.
    std::vector<Vec2i> verts{a, b, c};
    std::sort(verts.begin(),
              verts.end(),
              [] (auto a, auto b) { return a.y < b.y; });
    Vec2i &top = verts[2];
    Vec2i &mid = verts[1];
    Vec2i &bottom = verts[0];

    // Get the bounding boxes for the upper and lower portions of the
    // triangle.
    int totalHeight = top.y - bottom.y;
    assert(totalHeight > 0);
    float t = ((float)(mid.y - bottom.y) / totalHeight);
    int widthBetweenTopAndBottom = top.x - bottom.x;
    int xLerp = bottom.x + (int)(t * widthBetweenTopAndBottom);

    TriangleBoundingBoxes result;
    result.lowerMin = { std::min({ bottom.x, mid.x, xLerp }), bottom.y };
    result.lowerMax = { std::max({ bottom.x, mid.x, xLerp }), mid.y };
    result.upperMin = { std::min({ mid.x, top.x, xLerp }), std::min(mid.y + 1, top.y) };
    result.upperMax = { std::max({ mid.x, top.x, xLerp }), top.y };

    clamp(result.lowerMin, lowBound, highBound);
    clamp(result.lowerMax, lowBound, highBound);
    clamp(result.upperMin, lowBound, highBound);
    clamp(result.upperMax, lowBound, highBound);

    return result;
}

void fillTriangle(const Vec2i &a,
                  const Vec2i &b,
                  const Vec2i &c,
                  TGAImage &image,
                  const TGAColor &color)
{
    Vec2i ab = b - a;
    Vec2i ac = c - a;

    // If the triangle's vertices are collinear, then don't draw anything.
    if ((ab ^ ac).z == 0) {
        return;
    }

    Vec2i lowBound(0, 0);
    Vec2i highBound(image.get_width() - 1, image.get_height() - 1);
    TriangleBoundingBoxes bounds =
        getTriangleBoundingBoxes(a ,b ,c, lowBound, highBound);

    assert(bounds.lowerMin.x >= 0);
    assert(bounds.lowerMin.y >= 0);
    assert(bounds.lowerMax.x >= 0);
    assert(bounds.lowerMax.y >= 0);
    assert(bounds.upperMin.x <= image.get_width() - 1);
    assert(bounds.upperMin.y <= image.get_height() - 1);
    assert(bounds.upperMax.x <= image.get_width() - 1);
    assert(bounds.upperMax.y <= image.get_height() - 1);
    assert(bounds.lowerMax.y <= bounds.upperMin.y);
    assert(bounds.lowerMin.x <= bounds.lowerMax.x);
    assert(bounds.upperMin.x <= bounds.upperMax.x);
    assert(bounds.lowerMin.y <= bounds.lowerMax.y);
    assert(bounds.upperMin.y <= bounds.upperMax.y);

    Vec2i p;
    for (p.y = bounds.lowerMin.y; p.y <= bounds.lowerMax.y; p.y++) {
        for (p.x = bounds.lowerMin.x; p.x <= bounds.lowerMax.x; p.x++) {
            Vec2i ap(p - a);
            auto bCoords = barycentricCoords(ab, ac, ap);
            if (bCoords.u >= 0 &&
                bCoords.v >= 0 &&
                bCoords.w >= 0) {
                image.set(p.x, p.y, color);
            }
        }
    }
    for (p.y = bounds.upperMin.y; p.y <= bounds.upperMax.y; p.y++) {
        for (p.x = bounds.upperMin.x; p.x <= bounds.upperMax.x; p.x++) {
            Vec2i ap(p - a);
            auto bCoords = barycentricCoords(ab, ac, ap);
            if (bCoords.u >= 0 &&
                bCoords.v >= 0 &&
                bCoords.w >= 0.0) {
                image.set(p.x, p.y, color);
            }
        }
    }
}

void lesson2()
{
    const int width  = 800;
    const int height = 800;
    TGAImage image(width, height, TGAImage::RGB);

    for (int i = 0; i < model->numFaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f worldCoords[3];
        Vec2i screenCoords[3];
        for (int j = 0; j < 3; j++) {
            worldCoords[j] = model->vert(face[j]);
            screenCoords[j] =
                Vec2i((worldCoords[j].x + 1.0) * width / 2.0,
                      (worldCoords[j].y + 1.0) * height / 2.0);
        }

        Vec3f lightVec(0, 0, -1);
        Vec3f ab(worldCoords[1] - worldCoords[0]);
        Vec3f ac(worldCoords[2] - worldCoords[0]);
        Vec3f faceNormal = (ac ^ ab).normalized();
        float intensity = (faceNormal * lightVec) * 255.0;
        if (intensity > 0) { // Cull backfaces
            TGAColor faceIllum = TGAColor(intensity, intensity, intensity, 255);
            fillTriangle(screenCoords[0], screenCoords[1], screenCoords[2],
                         image,
                         faceIllum);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
}

void fill3DTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c,
                    std::vector<float> &zBuffer, TGAImage &image, TGAColor color)
{
    Vec3f ab(b - a);
    Vec3f ac(c - a);

    // If the triangle's vertices are collinear, then don't draw anything.
    if ((ab ^ ac).z == 0.0f) {
        return;
    }

    // Sort the vertices by y coord.
    std::vector<Vec3f> ySorted{ a, b, c };
    std::sort(ySorted.begin(), ySorted.end(),
              [] (auto a, auto b) { return a.y < b.y; });

    // Get the bounding boxes for the upper and lower halves of the triangle.
    float totalHeight = ySorted[2].y - ySorted[0].y;
    float t = (ySorted[1].y - ySorted[0].y) / totalHeight;
    float widthBetween0and2 = ySorted[2].x - ySorted[0].x;
    float xLerp = ySorted[0].x + (t * widthBetween0and2);
    Vec2i lowerBoxMin(int(std::min({ ySorted[0].x, ySorted[1].x, xLerp })),
                      int(ySorted[0].y));
    Vec2i lowerBoxMax(int(std::max({ ySorted[0].x, ySorted[1].x, xLerp })),
                      int(ySorted[1].y));
    Vec2i upperBoxMin(int(std::min({ ySorted[1].x, ySorted[2].x, xLerp })),
                      int(ySorted[1].y) + 1);
    Vec2i upperBoxMax(int(std::max({ ySorted[1].x, ySorted[2].x, xLerp })),
                      int(ySorted[2].y));

    Vec3f p;
    int width = image.get_width();
    for (p.y = lowerBoxMin.y; p.y <= lowerBoxMax.y; p.y++) {
        for (p.x = lowerBoxMin.x; p.x <= lowerBoxMax.x; p.x++) {
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
    for (p.y = upperBoxMin.y; p.y <= upperBoxMax.y; p.y++) {
        for (p.x = upperBoxMin.x; p.x <= upperBoxMax.x; p.x++) {
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
            fill3DTriangle(screenCoords[0], screenCoords[1], screenCoords[2],
                           zBuffer, image, faceIllum);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
}

int main(int argc, char** argv)
{
    // lesson0();
    // lesson1();
    // lesson2();
    lesson3();
    return 0;
}
