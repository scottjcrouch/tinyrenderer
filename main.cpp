#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>

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
    image.flip_vertically(); // Sets origin at the bottom-left corner of the image
    image.write_tga_file("output.tga");
}

void drawLine(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    // To draw the line without gaps, we have to iterate over the longest
    // coordinate.  Therefore we check here to see if the segment is "steep",
    // i.e. the line segment is taller than it is wide.  If so, we will
    // un-steepen temporarily by flipping the coordinates, then un-flip them
    // when we finally draw.
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

    // Normally each iteration, we would increment x by 1 and y by dy/dx, then
    // round y to the nearest pixel.  To save division ops, we pull the
    // calculation of dy/dx out of the loop (derror).  But we can further
    // eliminate needing to use floats altogether with some clever dimensional
    // analysis to eliminate some terms.
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
    const int width  = 800;
    const int height = 800;
    TGAImage image(width, height, TGAImage::RGB);

    // For each face, draw lines connecting all three points of the triangle.
    // The vertices are defined as floats in the range [-1.0, 1.0].  We obtain
    // pixel coords from them by scaling to the height and width of the tga
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

void drawTriangle(Vec2i a, Vec2i b, Vec2i c, TGAImage &image, TGAColor color)
{
    drawLine(a, b, image, color);
    drawLine(b, c, image, color);
    drawLine(c, a, image, color);
}

void drawSquare(Vec2i min, Vec2i max, TGAImage &image, TGAColor color)
{
    drawLine({min.x, min.y}, {min.x, max.y}, image, color);
    drawLine({min.x, min.y}, {max.x, min.y}, image, color);
    drawLine({max.x, max.y}, {min.x, max.y}, image, color);
    drawLine({max.x, max.y}, {max.x, min.y}, image, color);
}

void fillTriangle(Vec2i a, Vec2i b, Vec2i c, TGAImage &image, TGAColor color)
{
    Vec2i ab(b - a);
    Vec2i ac(c - a);

    // If the triangle's vertices are collinear, then we don't need to draw
    // anything.
    if ((ab ^ ac).z == 0) {
        return;
    }

    // Sort the vertices by y coord.
    std::vector<Vec2i> verts{a, b, c};
    std::sort(verts.begin(),
              verts.end(),
              [] (auto a, auto b) { return a.y < b.y; });

    // Get the bounding boxes for the upper and lower halves of the triangle.
    int totalHeight = verts[2].y - verts[0].y;
    float t = ((float)(verts[1].y - verts[0].y) / totalHeight);
    int widthBetween0and2 = verts[2].x - verts[0].x;
    bool signBit = widthBetween0and2 < 0;
    int xLerp = verts[0].x + (int)(t * widthBetween0and2 + (signBit ? -1 : 1));
    Vec2i lowerBoxMin(std::min({ verts[0].x, verts[1].x, xLerp }),
                      verts[0].y);
    Vec2i lowerBoxMax(std::max({ verts[0].x, verts[1].x, xLerp }),
                      verts[1].y);
    Vec2i upperBoxMin(std::min({ verts[1].x, verts[2].x, xLerp }),
                      verts[1].y + 1);
    Vec2i upperBoxMax(std::max({ verts[1].x, verts[2].x, xLerp }),
                      verts[2].y);

    Vec2i p;
    for (p.y = lowerBoxMin.y; p.y <= lowerBoxMax.y; p.y++) {
        for (p.x = lowerBoxMin.x; p.x <= lowerBoxMax.x; p.x++) {
            Vec2i ap(p-a);
            Vec2f bCoords = barycentricCoords(ab, ac, ap);
            if (bCoords.u >= 0 &&
                bCoords.v >= 0 &&
                bCoords.u + bCoords.v <= 1) {
                image.set(p.x, p.y, color);
            }
        }
    }
    for (p.y = upperBoxMin.y; p.y <= upperBoxMax.y; p.y++) {
        for (p.x = upperBoxMin.x; p.x <= upperBoxMax.x; p.x++) {
            Vec2i ap(p-a);
            Vec2f bCoords = barycentricCoords(ab, ac, ap);
            if (bCoords.u >= 0 &&
                bCoords.v >= 0 &&
                bCoords.u + bCoords.v <= 1) {
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

int main(int argc, char** argv)
{
    // lesson0();
    // lesson1();
    lesson2();
    return 0;
}
