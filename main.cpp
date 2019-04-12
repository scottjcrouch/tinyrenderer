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

// Draw a dot on a 100x100 px tga image
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

void fillTriangle(Vec2i a, Vec2i b, Vec2i c, TGAImage &image, TGAColor color)
{
    Vec2i ab(b-a);
    Vec2i ac(c-a);

    // If the triangle is degenerate, or its vertices are collinear, then
    // don't draw anything.
    if (crossProd(ac,ab).z <= 0) {
        return;
    }

    int xMin = std::min({ a.x, b.x, c.x });
    int yMin = std::min({ a.y, b.y, c.y });
    int xMax = std::max({ a.x, b.x, c.x });
    int yMax = std::max({ a.y, b.y, c.y });

    Vec2i p;
    for (p.y = yMin; p.y <= yMax; p.y++) {
        for (p.x = xMin; p.x <= xMax; p.x++) {
            Vec2i ap(p-a);
            Vec2f bcoord = barycentric(ab, ac, ap);
            if (bcoord.u >= 0 &&
                bcoord.v >= 0 &&
                bcoord.u + bcoord.v <= 1) {
                image.set(p.x, p.y, color);
            }
        }
    }
}

void lesson2()
{
    const int width  = 200;
    const int height = 200;
    TGAImage image(width, height, TGAImage::RGB);

    fillTriangle(Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80), image, red);
    fillTriangle(Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180), image, green);
    fillTriangle(Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180), image, blue);

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
