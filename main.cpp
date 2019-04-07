#include <cmath>

#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor black = TGAColor(  0,   0,   0, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(  0, 255,   0, 255);
const TGAColor blue  = TGAColor(  0,   0, 255, 255);

// Draw a dot on a 100x100 px tga image
void lesson0()
{
    TGAImage image(100, 100, TGAImage::RGB);
    image.set(50, 50, red);
    image.flip_vertically(); // Sets origin at the bottom-left corner of the image
    image.write_tga_file("output.tga");
}

void drawLine(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    // To draw the line without gaps, we have to iterate over the longest
    // coordinate.  Therefore we check here to see if the segment is "steep",
    // i.e. the line segment is taller than it is wide.  If so, we will
    // un-steepen temporarily by flipping the coordinates, then un-flip them
    // when we finally draw.
    bool isSteep = false;
    if (std::abs(x0-x1) < std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        isSteep = true;
    }

    // Ensure the leftmost point is {x0,y0}.
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    // Normally each iteration, we would increment x by 1 and y by dy/dx, then
    // round y to the nearest pixel.  To save division ops, we pull the
    // calculation of dy/dx out of the loop (derror).  But we can further
    // eliminate needing to use floats altogether with some clever dimensional
    // analysis to eliminate some terms.
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    for (int x = x0, y = y0; x <= x1; x++) {
        if (isSteep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += ((y1 > y0) ? 1 : -1);
            error2 -= (dx * 2);
        }
    }
}

// Draw a series of lines to draw a mesh
void lesson1()
{
    TGAImage image(100, 100, TGAImage::RGB);
    drawLine(20, 13, 40, 80, image, red);
    drawLine(80, 40, 13, 20, image, green);
    image.flip_vertically();
    image.write_tga_file("output.tga");
}

int main(int argc, char** argv)
{
    // lesson0();
    lesson1();
    return 0;
}
