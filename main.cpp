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

Vec3f lightVec = Vec3f(1, 1, 1).normalized();
Vec3f origin(0, 0, 0);
Vec3f eye(1, 1, 3);
Vec3f up(0, 1, 0);

struct GouraudShader : public IShader {
    Vec3f varyingIntensity;
    std::array<Vec3f, 3> textureVertices;

    virtual Vec3f vertex(int faceIndex, int vertexIndex) {
        std::vector<int> face = model.getFace(faceIndex);

        varyingIntensity.raw[vertexIndex] =
            std::max(0.f, model.getVertexNormal(face[vertexIndex*3 + 2]) * lightVec);
        textureVertices[vertexIndex] =
            model.getTextureVertex(face[vertexIndex*3 + 1]);
        textureVertices[vertexIndex].x *= texture.get_width();
        textureVertices[vertexIndex].y *= texture.get_height();

        Vec3f glVertex = model.getVertex(face[vertexIndex*3]);
        return viewPort * projection * modelView * glVertex;
    }

    virtual bool fragment(const Vec3f &baryCoords, TGAColor &color) {
        float intensity = varyingIntensity * baryCoords;
        assert(intensity >= 0.0 && intensity <= 1.0);
        Vec3f texel =
            textureVertices[0] * baryCoords.u +
            textureVertices[1] * baryCoords.v +
            textureVertices[2] * baryCoords.w;
        color = texture.get(int(texel.x), int(texel.y));
        color.r *= intensity;
        color.g *= intensity;
        color.b *= intensity;

        return false;
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
