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

void drawTriangle(std::array<Vec2i,3> verts, TGAImage &image, TGAColor color)
{
    drawLine(verts[0], verts[1], image, color);
    drawLine(verts[1], verts[2], image, color);
    drawLine(verts[2], verts[0], image, color);
}

void fillTriangle(std::array<Vec2i,3> verts, TGAImage &image, TGAColor color)
{
    // Sort the vertices by y and x coords.
    std::sort(verts.begin(),
              verts.end(),
              [] (auto v0, auto v1) {
                  if (v0.y == v1.y) {
                      return v0.x < v1.x;
                  } else {
                      return v0.y < v1.y;
                  }
              });

    // If any vertices overlap, then the area of the triangle is zero, so we
    // don't need to draw anything and can return early.
    if (std::adjacent_find(verts.begin(), verts.end()) != verts.end()) {
        return;
    }

    // Get the cross product of vectors of vert0 -> vert1 and vert 0 -> vert
    // 2.  We will use this to check whether the angle between them is
    // positive, negative or 0.
    auto crossP = cross(verts[1] - verts[0], verts[2] - verts[0]);

    // If the three vertices are collinear, then the area of the triangle is
    // zero, so we don't need to draw anything.
    if (crossP.z == 0) {
        return;
    }

    // Since the order of the points should not affect the fill, we must first
    // rearrange the order of the three vertices so that the right hand side
    // of each line connecting them is always the triangle interior.
    if (crossP.z < 0) {
        std::swap(verts[1], verts[2]);
    }

    // For each pixel, we will fill it only if it lies on the inside of the
    // three edges.  Thanks to the code above, we can assume that the right
    // hand side of the vector defining each edge is the inside.  In this
    // case, to test whether a pixel is on the inside of all three edges, we
    // compute the dot product of the perpendicular for each edge * the vector
    // made by the pixel.

    // Get the edge vectors.
    Vec2i vec0(verts[1] - verts[0]);
    Vec2i vec1(verts[2] - verts[1]);
    Vec2i vec2(verts[0] - verts[2]);

    // Get their perpendiculars.
    Vec2i perp0(perp(vec0));
    Vec2i perp1(perp(vec1));
    Vec2i perp2(perp(vec2));

    // Get the bounds of the square the triangle lies inside.
    int xMin = std::min({ verts[0].x, verts[1].x, verts[2].x });
    int yMin = std::min({ verts[0].y, verts[1].y, verts[2].y });
    int xMax = std::max({ verts[0].x, verts[1].x, verts[2].x });
    int yMax = std::max({ verts[0].y, verts[1].y, verts[2].y });

    // TODO: Start filling!
    for (int y = yMin; y <= yMax; y++) {
        for (int x = xMin; x <= xMax; x++) {
            if (dot(perp0, Vec2i(x, y) - verts[0]) > 0 &&
                dot(perp1, Vec2i(x, y) - verts[1]) > 0 &&
                dot(perp2, Vec2i(x, y) - verts[2]) > 0) {
                image.set(x, y, color);
            }
        }
    }
}

void lesson2()
{
    const int width  = 200;
    const int height = 200;
    TGAImage image(width, height, TGAImage::RGB);

    std::array<Vec2i,3>t0{Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    std::array<Vec2i,3>t1{Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    std::array<Vec2i,3>t2{Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    fillTriangle(t0, image, red);
    fillTriangle(t1, image, green);
    fillTriangle(t2, image, blue);

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
