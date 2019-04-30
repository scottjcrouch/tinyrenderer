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

void fillTriangle(const std::array<Vec3f, 3> &vertices,
                  const std::array<Vec3f, 3> &textureVertices,
                  const std::array<float, 3> &intensities,
                  std::vector<float> &zBuffer,
                  TGAImage &image,
                  TGAImage &texture)
{
    const Vec3f &a = vertices[0];
    const Vec3f &b = vertices[1];
    const Vec3f &c = vertices[2];

    Vec3f ab(b - a);
    Vec3f ac(c - a);

    // Ensure triangle is not degenerate.
    assert((ab ^ ac).z != 0.0f);

    // Get the on-screen bounding box for the triangle.
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
            p.z = a.z*bary.w + b.z*bary.u + c.z*bary.v;
            if (zBuffer[int(p.y*width + p.x)] >= p.z) {
                continue;
            }
            zBuffer[int(p.y*width + p.x)] = p.z;
            Vec3f texel = textureVertices[0] * bary.w +
                         textureVertices[1] * bary.u +
                         textureVertices[2] * bary.v;
            TGAColor color = texture.get(int(texel.x), int(texel.y));
            float intensity = intensities[0] * bary.w +
                         intensities[1] * bary.u +
                         intensities[2] * bary.v;
            if (intensity < 0.0f) {
                image.set(p.x, p.y, black);
            } else {
                color.r *= intensity;
                color.g *= intensity;
                color.b *= intensity;
                image.set(p.x, p.y, color);
            }
        }
    }
}

void lesson5()
{
    Model model("obj/african_head.obj");
    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();
    constexpr int width = 800, height = 800, depth = 255;
    TGAImage image(width, height, TGAImage::RGB);
    std::vector<float> zBuffer(image.get_width() * image.get_height(),
                               std::numeric_limits<float>::lowest());

    Vec3f cameraPos(0, 0, 1.5);

    Matrix projection;
    projection[3][2] = -1.0f / cameraPos.z;
    Matrix translate;
    translate[0][3] = 1.0f;
    translate[1][3] = 1.0f;
    translate[2][3] = 1.0f;
    Matrix stretch;
    stretch[0][0] = width / 2.0f;
    stretch[1][1] = height / 2.0f;
    stretch[2][2] = depth / 2.0f;

    Vec3f lightVec(0, 0, -1);

    for (int i = 0; i < model.numFaces(); i++) {
        std::vector<int> face = model.getFace(i);
        std::array<Vec3f, 3> faceVertices;
        std::array<Vec3f, 3> textureVertices;
        std::array<Vec3f, 3> vertexNormals;
        std::array<Vec3f, 3> screenCoords;
        std::array<Vec3f, 3> textureCoords;
        std::array<float, 3> intensities;

        for (int j = 0; j < 3; j++) {
            faceVertices[j] = model.getVertex(face[j*3]);
            textureVertices[j] = model.getTextureVertex(face[j*3 + 1]);
            vertexNormals[j] = model.getVertexNormal(face[j*3 + 2]);

            faceVertices[j] = projection * faceVertices[j];
            screenCoords[j] = stretch * (translate * faceVertices[j]);

            textureCoords[j] = { textureVertices[j].x * texture.get_width(),
                                 textureVertices[j].y * texture.get_height(),
                                 textureVertices[j].z };
            intensities[j] = -(vertexNormals[j] * lightVec);
        }

        /* Backface culling. */
        Vec3f ab(faceVertices[1] - faceVertices[0]);
        Vec3f ac(faceVertices[2] - faceVertices[0]);
        if ((ab ^ ac) * cameraPos <= 0) {
            continue;
        }

        fillTriangle(screenCoords, textureCoords, intensities,
                     zBuffer, image, texture);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
}

int main(int argc, char** argv)
{
    lesson5();
    return 0;
}
