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
TGAImage outputImage(width, height, TGAImage::RGB);
std::vector<float> zBuf(width * height, std::numeric_limits<float>::lowest());
std::vector<float> shadowBuf(width * height, std::numeric_limits<float>::lowest());

Vec3f lightVec = Vec3f(1, 1, 1).normalized();

Vec3f origin(0, 0, 0);
Vec3f eye(1, 1, 3);
Vec3f up(0, 1, 0);

struct PhongShader : public IShader
{
    Matrix2x3 vertexUVs;
    Matrix3x3 vertexNormals;
    Matrix3x3 vertexCoords;
    Matrix4x4 M;
    Matrix4x4 MIT;
    Matrix4x4 Mshadow;
    Vec3f light;

    virtual Vec3f vertex(int faceIndex, int vertexIndex) {
        // Fetch vertex data from the model.
        Vec3f vertex = model->getVertex(faceIndex, vertexIndex);
        Vec3f normal = model->getVertexNormal(faceIndex, vertexIndex);
        Vec2f uv = model->getTextureVertex(faceIndex, vertexIndex);

        // Transform the vertex and normal to our perspective.
        vertex = M * vertex;
        normal = MIT * normal;

        // Record data needed by the fragment shader.
        vertexCoords.setCol(vertexIndex, vertex);
        vertexNormals.setCol(vertexIndex, normal);
        vertexUVs.setCol(vertexIndex, uv);

        // Return the position on the display where the vertex projects.
        return viewport * vertex;
    }

    virtual bool fragment(const Vec3f &barycentricCoords, TGAColor &color) {
        Vec2f uv = vertexUVs * barycentricCoords;
        TGAColor textureColor = model->getTextureColor(uv);
        Vec3f objectSpaceNormal = (vertexNormals * barycentricCoords).normalized();
        Vec3f tangentSpaceNormal = model->getTangentNormal(uv);

        Vec3f globalCoord = (vertexCoords * barycentricCoords);
        Vec3f shadowBufCoord = Mshadow * globalCoord;
        int shadowBufIndex = int(shadowBufCoord.x) + int(shadowBufCoord.y)*width;
        float zFightingMagicNum = 43.34;
        bool occluded = shadowBuf[shadowBufIndex] > (shadowBufCoord.z + zFightingMagicNum);
        float shadow = 0.3f + (occluded ? 0.0f : 0.7f);

        Matrix3x3 A;
        A.setRow(0, vertexCoords.getCol(1) - vertexCoords.getCol(0));
        A.setRow(1, vertexCoords.getCol(2) - vertexCoords.getCol(0));
        A.setRow(2, objectSpaceNormal);
        Matrix3x3 AI = A.inverse();
        Vec2f uv0 = vertexUVs.getCol(0);
        Vec2f uv1 = vertexUVs.getCol(1);
        Vec2f uv2 = vertexUVs.getCol(2);
        Vec3f i = (AI * Vec3f(uv1.u-uv0.u, uv2.u-uv0.u, 0));
        Vec3f j = (AI * Vec3f(uv1.v-uv0.v, uv2.v-uv0.v, 0));
        Matrix3x3 tangentBasis;
        tangentBasis.setCol(0, i.normalized());
        tangentBasis.setCol(1, j.normalized());
        tangentBasis.setCol(2, objectSpaceNormal);
        Vec3f normal = (tangentBasis * tangentSpaceNormal).normalized();

        float diffuseIntensity = std::max(normal * light, 0.0f);
        assert(diffuseIntensity <= 1.0f);

        float specularPower = model->getSpecularPower(uv);
        Vec3f reflection = (-light + normal*(normal*light)*2).normalized();
        float specularIntensity = std::pow(std::max(reflection.z, 0.0f), specularPower);
        assert(specularIntensity <= 1.0f);

        for (int i = 0; i < 3; i++) {
            float intensity = 0.2f + shadow * (0.8f*diffuseIntensity + 0.4f*specularIntensity);
            color.raw[i] = std::min(textureColor.raw[i] * intensity, 255.0f);
        }

        // Specify not to discard this fragment.
        return false;
    }
};

struct DepthShader : public IShader
{
    Matrix3x3 vertexCoords;
    Matrix4x4 M;

    virtual Vec3f vertex(int faceIndex, int vertexIndex) {
        Vec3f vertex = model->getVertex(faceIndex, vertexIndex);
        vertex = M * vertex;
        vertexCoords.setCol(vertexIndex, vertex);
        return vertex;
    }

    virtual bool fragment(const Vec3f& barycentricCoords, TGAColor &color) {
        Vec3f p = vertexCoords * barycentricCoords;
        color = TGAColor(255, 255, 255) * (p.z / depth);
        return false;
    }
};

int main(int argc, char** argv)
{
    /*
     * First pass where we populate the shadow buffer with depth values at
     * each point where the light casts.
     */

    lookAt(lightVec, origin, up); // Put camera at position of light source.
    view(width/8, height/8, width*3/4, height*3/4);
    project(0); // Set infinite focal length (orthogonal projection)

    DepthShader depthShader;
    depthShader.M = viewport * projection * modelview;

    for (int faceIndex = 0; faceIndex < model->numFaces(); faceIndex++) {
        std::array<Vec3f, 3> screenCoords;

        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            screenCoords[vertexIndex] = depthShader.vertex(faceIndex, vertexIndex);
        }

        drawTriangle(screenCoords, depthShader, outputImage, shadowBuf);
    }

    outputImage.flip_vertically();
    outputImage.write_tga_file("depth.tga");
    outputImage.clear();

    /*
     * Second pass where we do our final render using said shadow buffer.
     */

    lookAt(eye, origin, up);
    view(width/8, height/8, width*3/4, height*3/4);
    project(-1.0f / (eye-origin).magnitude());

    PhongShader shader;
    shader.M = projection * modelview;
    shader.MIT = (projection * modelview).inverseTranspose();
    shader.Mshadow = depthShader.M * shader.M.inverse();
    shader.light = (projection * modelview * lightVec).normalized();

    for (int faceIndex = 0; faceIndex < model->numFaces(); faceIndex++) {
        std::array<Vec3f, 3> screenCoords;

        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            screenCoords[vertexIndex] = shader.vertex(faceIndex, vertexIndex);
        }

        drawTriangle(screenCoords, shader, outputImage, zBuf);
    }

    outputImage.flip_vertically();
    outputImage.write_tga_file("output.tga");
    outputImage.clear();

    return 0;
}
