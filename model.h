#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>

#include "geometry.h"
#include "tgaimage.h"

class Model
{
public:
    Model(std::string path);

    int numFaces();
    std::vector<int> getFace(int index);
    Vec3f getVertex(int faceIndex, int vertexIndex);
    Vec2f getTextureVertex(int faceIndex, int vertexIndex);
    Vec3f getVertexNormal(int faceIndex, int vertexIndex);

    TGAImage diffuseMap;
    TGAImage normalMap;
    TGAImage specularMap;

    TGAColor getTextureColor(Vec2f uv);
    Vec3f getTextureNormal(Vec2f uv);
    float getSpecularPower(Vec2f uv);

private:
    bool loadObj(std::string filename);
    bool loadDiffuseMap(std::string filename);
    bool loadNormalMap(std::string filename);
    bool loadSpecularMap(std::string filename);

    std::vector<std::vector<int>> faces;
    std::vector<Vec3f> vertices;
    std::vector<Vec2f> textureVertices;
    std::vector<Vec3f> vertexNormals;
};

#endif // __MODEL_H__
