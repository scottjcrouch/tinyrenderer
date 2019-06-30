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

auto model(std::make_unique<Model>("obj/african_head"));

constexpr int width = 800, height = 800, depth = 255;
TGAImage output(width, height, TGAImage::RGB);
std::vector<float> zBuffer(width * height, std::numeric_limits<float>::lowest());

Vec3f origin(0, 0, 0);
Vec3f lightVec = Vec3f(1, 1, 0).normalized();
Vec3f eye(1, 1, 4);
Vec3f up(0, 1, 0);

struct GouraudShader : public IShader {
    std::array<Vec2f, 3> textureVertices;
    Matrix uniform_projModelview;
    Matrix uniform_projModelviewIT;
    Matrix uniform_viewportProjModelview;

    virtual Vec3f vertex(int faceIndex, int vertexIndex) {
        textureVertices[vertexIndex] = model->getTextureVertex(faceIndex, vertexIndex);
        Vec3f glVertex = model->getVertex(faceIndex, vertexIndex);
        return uniform_viewportProjModelview * glVertex;
    }

    virtual bool fragment(const Vec3f &baryCoords, TGAColor &color) {
        Vec2f texel =
            textureVertices[0] * baryCoords.u +
            textureVertices[1] * baryCoords.v +
            textureVertices[2] * baryCoords.w;

        Vec3f textureNormal = model->getTextureNormal(texel);
        float intensity =
            (uniform_projModelviewIT * textureNormal).normalized() *
            (uniform_projModelview * lightVec).normalized();
        assert(intensity <= 1.0f);
        if (intensity < 0.0f)
            intensity = 0.0f;

        color = model->getTextureColor(texel) * intensity;

        return false;
    }
};

void lesson6()
{
    lookAt(eye, origin, up);
    view(0, 0, width, height);
    project(-1.0f / (eye-origin).magnitude());

    GouraudShader shader;
    shader.uniform_projModelview = projection * modelview;
    shader.uniform_projModelviewIT = (projection * modelview).inverse().transpose();
    shader.uniform_viewportProjModelview = viewport * projection * modelview;

    for (int faceIndex = 0; faceIndex < model->numFaces(); faceIndex++) {
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
