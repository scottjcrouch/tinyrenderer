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

void fillTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c,
                  const Vec3f &ta, const Vec3f &tb, const Vec3f &tc,
                  std::vector<float> &zBuffer, TGAImage &image, float intensity,
                  TGAImage &texture)
{
    Vec3f ab(b - a);
    Vec3f ac(c - a);

    if ((ab ^ ac).z != 0.0f) {
        // Degenerate triangle.
        return;
    }

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
            Vec3f t = ta*bCoords.w + tb*bCoords.u + tc*bCoords.v;
            TGAColor color = texture.get(int(t.x), int(t.y));
            color.r *= intensity;
            color.g *= intensity;
            color.b *= intensity;
            image.set(p.x, p.y, color);
        }
    }
}

void lesson4()
{
    Model model("obj/african_head.obj");
    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();
    TGAImage image(800, 800, TGAImage::RGB);
    std::vector<float> zBuffer(image.get_width() * image.get_height(),
                               std::numeric_limits<float>::lowest());
    Vec3f cameraPos(0, 0, 3);
    Vec3f lightVec(0, 0, -1);

    for (int i = 0; i < model.numFaces(); i++) {
        std::vector<int> face = model.getFace(i);
        Vec3f faceVertices[3];
        Vec3f textureVertices[3];
        Vec3f vertexNormals[3];
        Vec3f screenCoords[3];
        Vec3f textureCoords[3];

        for (int j = 0; j < 3; j++) {
            faceVertices[j] = model.getVertex(face[j*3]);
            textureVertices[j] = model.getTextureVertex(face[j*3 + 1]);
            vertexNormals[j] = model.getVertexNormal(face[j*3 + 2]);

            faceVertices[j] = faceVertices[j] * (1/(1 - (faceVertices[j].z/cameraPos.z)));
            screenCoords[j] =
                Vec3f((faceVertices[j].x + 1.0) * image.get_width() / 2.0,
                      (faceVertices[j].y + 1.0) * image.get_height() / 2.0,
                      faceVertices[j].z);
            textureCoords[j] =
                Vec3f(textureVertices[j].x * texture.get_width(),
                      textureVertices[j].y * texture.get_height(),
                      textureVertices[j].z);
        }

        /* Lighting and backface culling. */
        Vec3f ab(faceVertices[1] - faceVertices[0]);
        Vec3f ac(faceVertices[2] - faceVertices[0]);
        if ((ab ^ ac).z != 0.0f) {
            // Degenerate triangle.
            return;
        }
        Vec3f faceNormal = (ac ^ ab).normalized();
        float intensity = faceNormal * lightVec;
        if (intensity <= 0) {
            // Backface.
            continue;
        }

        fillTriangle(screenCoords[0], screenCoords[1], screenCoords[2],
                     textureCoords[0], textureCoords[1], textureCoords[2],
                     zBuffer, image, intensity, texture);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
}

int main(int argc, char** argv)
{
    lesson4();
    return 0;
}
