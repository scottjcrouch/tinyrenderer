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
Vec3f lightVec = Vec3f(1, 1, 1).normalized();
Vec3f eye(1, 1, 3);
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

        TGAColor textureColor = model->getTextureColor(texel);

        // The actual "transpose" portion of the inverse transpose happens
        // here, by representing the normal as a column (vector) instead of a
        // row.
        Vec3f textureNormal = model->getTextureNormal(texel);
        Vec3f transformedNormal = (uniform_projModelviewIT * textureNormal).normalized();
        Vec3f transformedLightVec = (uniform_projModelview * lightVec).normalized();
        float diffuseIntensity = transformedNormal * transformedLightVec;
        assert(diffuseIntensity <= 1.0f);
        if (diffuseIntensity < 0.0f)
            diffuseIntensity = 0.0f;
        TGAColor diffuseColor = textureColor * diffuseIntensity;

        float specularPower = model->getSpecularPower(texel);
        Vec3f reflection = (transformedNormal*2 - transformedLightVec).normalized();
        float specularIntensity = std::pow(std::max(reflection.z, 0.0f), specularPower);
        assert(specularIntensity <= 1.0f);
        if (specularIntensity < 0.0f)
            specularIntensity = 0.0f;
        TGAColor specularColor = textureColor * specularIntensity;

        constexpr float ambientCoeff = 0.1f;
        constexpr float diffuseCoeff = 1.0f;
        constexpr float specularCoeff = 0.6f;
        for (int i = 0; i < 3; i++) {
            color.raw[i] =
                std::min(textureColor.raw[i] * ambientCoeff +
                         diffuseColor.raw[i] * diffuseCoeff +
                         specularColor.raw[i] * specularCoeff,
                         255.0f);
        }

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
    shader.uniform_projModelviewIT = (projection * modelview).inverse();
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
