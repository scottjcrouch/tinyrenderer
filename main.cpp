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

struct GouraudShader : public IShader
{
    Matrix2x3 vertexUVs;
    Matrix3x3 vertexNormals;
    Matrix3x3 vertexCoords;
    Matrix4x4 uniform_M;
    Matrix4x4 uniform_MIT;

    virtual Vec3f vertex(int faceIndex, int vertexIndex) {
        // Fetch vertex data from the model.
        Vec3f vertex = model->getVertex(faceIndex, vertexIndex);
        Vec3f normal = model->getVertexNormal(faceIndex, vertexIndex);
        Vec2f uv = model->getTextureVertex(faceIndex, vertexIndex);

        // Transform the vertex and normal.
        vertex = uniform_M * vertex;
        normal = uniform_MIT * normal;

        // Record data for the fragment shader.
        vertexCoords.setCol(vertex, vertexIndex);
        vertexNormals.setCol(normal, vertexIndex);
        vertexUVs.setCol(uv, vertexIndex);

        // Return the position on the display where the vertex projects.
        return viewport * vertex;
    }

    virtual bool fragment(const Vec3f &barycentricCoords, TGAColor &color) {
        Vec2f uv = vertexUVs * barycentricCoords;
        TGAColor textureColor = model->getTextureColor(uv);
        Vec3f objectSpaceNormal = (vertexNormals * barycentricCoords).normalized();
        Vec3f tangentSpaceNormal = model->getTangentNormal(uv);
        Vec3f transformedLightVec = (uniform_M * lightVec).normalized();

        Matrix3x3 A;
        A.setRow(vertexCoords.getCol(1) - vertexCoords.getCol(0), 0);
        A.setRow(vertexCoords.getCol(2) - vertexCoords.getCol(0), 1);
        A.setRow(objectSpaceNormal, 2);
        Matrix3x3 AI = A.inverse();
        Vec2f uv0 = vertexUVs.getCol(0);
        Vec2f uv1 = vertexUVs.getCol(1);
        Vec2f uv2 = vertexUVs.getCol(2);
        Matrix3x3 tangentBasis;
        Vec3f i = (AI * Vec3f(uv1.u-uv0.u, uv2.u-uv0.u, 0));
        Vec3f j = (AI * Vec3f(uv1.v-uv0.v, uv2.v-uv0.v, 0));
        tangentBasis.setCol(i, 0);
        tangentBasis.setCol(j, 1);
        tangentBasis.setCol(objectSpaceNormal, 2);
        Vec3f normal = (tangentBasis * tangentSpaceNormal).normalized();

        float diffuseIntensity = normal * transformedLightVec;
        assert(diffuseIntensity <= 1.0f);
        if (diffuseIntensity < 0.0f)
            diffuseIntensity = 0.0f;
        TGAColor diffuseColor = textureColor * diffuseIntensity;

        // Vec3f textureNormal = model->getTextureNormal(uv);
        // Vec3f normal = (uniform_MIT * textureNormal).normalized();
        // float diffuseIntensity = normal * transformedLightVec;
        // assert(diffuseIntensity <= 1.0f);
        // if (diffuseIntensity < 0.0f)
        //     diffuseIntensity = 0.0f;
        // TGAColor diffuseColor = textureColor * diffuseIntensity;

        float specularPower = model->getSpecularPower(uv);
        Vec3f reflection = (normal*2 - transformedLightVec).normalized();
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

        // Specify not to discard this fragment.
        return false;
    }
};

int main(int argc, char** argv)
{
    lookAt(eye, origin, up);
    view(0, 0, width, height);
    project(-1.0f / (eye-origin).magnitude());

    GouraudShader shader;
    shader.uniform_M = projection * modelview;
    // Note that the "transpose" portion of the inverse transpose happens
    // implicitly when we perform the matrix multiplication on normals in the
    // fragment shader.  This is because we represent each normal as a column
    // vector, rather than a row as it appears in the IT equation.
    shader.uniform_MIT = (projection * modelview).inverse();

    for (int faceIndex = 0; faceIndex < model->numFaces(); faceIndex++) {
        std::array<Vec3f, 3> screenCoords;

        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            screenCoords[vertexIndex] = shader.vertex(faceIndex, vertexIndex);
        }

        drawTriangle(screenCoords, shader, output, zBuffer);
    }

    output.flip_vertically();
    output.write_tga_file("output.tga");

    return 0;
}
