#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "gl.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor black = TGAColor(  0,   0,   0, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(  0, 255,   0, 255);
const TGAColor blue  = TGAColor(  0,   0, 255, 255);

Model model;
TGAImage texture;

constexpr int width = 800, height = 800, depth = 255;
TGAImage output(width, height, TGAImage::RGB);
std::vector<float> zBuffer(width * height, std::numeric_limits<float>::lowest());

Vec3f lightVec(0, 0, 1);
Vec3f origin(0, 0, 0);
// Vec3f eye(1, 1, 3);
Vec3f eye(0, 0, 3);
Vec3f up(0, 1, 0);

struct GouraudShader : public IShader {
    Vec3f varyingIntensity; // written by vertex shader, read by fragment shader

    virtual Vec3f vertex(int faceIndex, int vertexIndex) {
        std::vector<int> face = model.getFace(faceIndex);
        varyingIntensity.raw[vertexIndex] =
            std::max(0.f, model.getVertexNormal(face[vertexIndex*3 + 2]) * lightVec);
        Vec3f glVertex = model.getVertex(face[vertexIndex*3]);
        return viewPort * projection * modelView * glVertex;
    }

    virtual bool fragment(const Vec3f &baryCoords, TGAColor &color) {
        float intensity = varyingIntensity * baryCoords;
        color.r = 255 * intensity;
        color.g = 255 * intensity;
        color.b = 255 * intensity;
        return false; // don't discard this pixel

        // float intensity = varyingIntensity*baryCoords;
        // if (intensity>.85) intensity = 1;
        // else if (intensity>.60) intensity = .80;
        // else if (intensity>.45) intensity = .60;
        // else if (intensity>.30) intensity = .45;
        // else if (intensity>.15) intensity = .30;
        // else intensity = 0;
        // color.r = 255 * intensity;
        // color.g = 155 * intensity;
        // color.b = 0;
        // return false;
    }
};

void lesson5()
{
    model.readFile("obj/african_head.obj");
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();

    lookAt(eye, origin, up);
    view(0, 0, width, height);
    project(-1.0f / (eye-origin).magnitude());

    GouraudShader shader;

    for (int faceIndex = 0; faceIndex < model.numFaces(); faceIndex++) {
        std::array<Vec3f, 3> screenCoords;

        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            screenCoords[vertexIndex] = shader.vertex(faceIndex, vertexIndex);
        }

        drawTriangle(screenCoords, shader, output, zBuffer);
    }

    output.flip_vertically();
    output.write_tga_file("output.tga");
}

int main(int argc, char** argv)
{
    lesson5();
    return 0;
}
