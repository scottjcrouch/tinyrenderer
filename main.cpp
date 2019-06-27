#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>
#include <iostream>

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
TGAImage normalMap;

constexpr int width = 800, height = 800, depth = 255;
TGAImage output(width, height, TGAImage::RGB);
std::vector<float> zBuffer(width * height, std::numeric_limits<float>::lowest());

Vec3f   origin(0, 0, 0);

Vec3f lightVec(1, 1, 0);
Vec3f      eye(1, 1, 4);
Vec3f       up(0, 1, 0);

struct GouraudShader : public IShader {
    std::array<Vec3f, 3> textureVertices;
    Matrix uniform_projModelview;
    Matrix uniform_projModelviewIT;
    Matrix uniform_viewportProjModelview;

    virtual Vec3f vertex(int faceIndex, int vertexIndex) {
        std::vector<int> face = model.getFace(faceIndex);

        textureVertices[vertexIndex] =
            model.getTextureVertex(face[vertexIndex*3 + 1]);
        textureVertices[vertexIndex].x *= texture.get_width();
        textureVertices[vertexIndex].y *= texture.get_height();

        Vec3f glVertex = model.getVertex(face[vertexIndex*3]);
        return uniform_viewportProjModelview * glVertex;
    }

    virtual bool fragment(const Vec3f &baryCoords, TGAColor &color) {
        Vec3f texel =
            textureVertices[0] * baryCoords.u +
            textureVertices[1] * baryCoords.v +
            textureVertices[2] * baryCoords.w;
        color = texture.get(int(texel.x), int(texel.y));

        TGAColor normalMapColor = normalMap.get(int(texel.x), int(texel.y));
        Vec3f vertexNormal((normalMapColor.r / 255.0f) * 2.0f - 1.0f,
                           (normalMapColor.g / 255.0f) * 2.0f - 1.0f,
                           (normalMapColor.b / 255.0f) * 2.0f - 1.0f);

        float intensity =
            (uniform_projModelviewIT * vertexNormal).normalized() *
            (uniform_projModelview * lightVec).normalized();
        assert(intensity <= 1.0f);
        if (intensity < 0.0f) {
            intensity = 0.0;
        }

        color.r *= intensity;
        color.g *= intensity;
        color.b *= intensity;

        return false;
    }
};

void lesson6()
{
    model.readFile("obj/african_head.obj");
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();
    normalMap.read_tga_file("obj/african_head_nm.tga");
    normalMap.flip_vertically();

    lookAt(eye, origin, up);
    view(0, 0, width, height);
    project(-1.0f / (eye-origin).magnitude());

    GouraudShader shader;
    shader.uniform_projModelview = projection * modelview;
    shader.uniform_projModelviewIT = (projection * modelview).inverse().transpose();
    shader.uniform_viewportProjModelview = viewport * projection * modelview;

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
    lesson6();
    return 0;
}
