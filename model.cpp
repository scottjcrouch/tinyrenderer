#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "model.h"
#include "tgaimage.h"

Model::Model(std::string path)
{
    if (!loadObj(path + ".obj"))
        assert(0);
    if (!loadDiffuseMap(path + "_diffuse.tga"))
        assert(0);
    if (!loadNormalMap(path + "_nm.tga"))
        assert(0);
    if (!loadTangentMap(path + "_nm_tangent.tga"))
        assert(0);
    if (!loadSpecularMap(path + "_spec.tga"))
        assert(0);
}

bool Model::loadObj(std::string filename)
{
    std::ifstream in;
    in.open(filename.c_str(), std::ifstream::in);
    if (in.fail()) {
        return false;
    }

    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char dummy_char;
        if (!line.compare(0, 2, "v ")) {
            iss >> dummy_char;
            Vec3f vertex;
            for (int i = 0; i < 3; i++)
                iss >> vertex.raw[i];
            vertices.push_back(vertex);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> face;
            int vertexIndex, textureVertexIndex, vertexNormalIndex;
            iss >> dummy_char;
            for (int i = 0; i < 3; ++i) {
                iss >> vertexIndex >> dummy_char
                    >> textureVertexIndex >> dummy_char
                    >> vertexNormalIndex;
                // we decrement because wavefront .obj indices start at 1, not 0
                face.push_back(--vertexIndex);
                face.push_back(--textureVertexIndex);
                face.push_back(--vertexNormalIndex);
            }
            faces.push_back(std::move(face));
        } else if (!line.compare(0, 3, "vt ")) {
            float u, v, w;
            iss >> dummy_char >> dummy_char >> u >> v >> w;
            textureVertices.emplace_back(u, v);
        } else if (!line.compare(0, 3, "vn ")) {
            float i, j, k;
            iss >> dummy_char >> dummy_char >> i >> j >> k;
            vertexNormals.emplace_back(i, j, k);
        }
    }

    return true;
}

bool Model::loadDiffuseMap(std::string path)
{
    return
        diffuseMap.read_tga_file(path.c_str()) &&
        diffuseMap.flip_vertically();
}

bool Model::loadNormalMap(std::string path)
{
    return
        normalMap.read_tga_file(path.c_str()) &&
        normalMap.flip_vertically();
}

bool Model::loadTangentMap(std::string path)
{
    return
        tangentMap.read_tga_file(path.c_str()) &&
        tangentMap.flip_vertically();
}

bool Model::loadSpecularMap(std::string path)
{
    return
        specularMap.read_tga_file(path.c_str()) &&
        specularMap.flip_vertically();
}

int Model::numFaces()
{
    return faces.size();
}

Vec3f Model::getVertex(int faceIndex, int vertexIndex)
{
    assert(faceIndex >= 0 && faceIndex < (int)faces.size());
    assert(vertexIndex >= 0 && vertexIndex < 3);
    int index = faces[faceIndex][vertexIndex*3];
    assert(index >= 0 && index < (int)vertices.size());
    return vertices[index];
}

Vec2f Model::getTextureVertex(int faceIndex, int vertexIndex)
{
    assert(faceIndex >= 0 && faceIndex < (int)faces.size());
    assert(vertexIndex >= 0 && vertexIndex < 3);
    int index = faces[faceIndex][vertexIndex*3 + 1];
    assert(index >= 0 && index < (int)textureVertices.size());
    return textureVertices[index];
}

Vec3f Model::getVertexNormal(int faceIndex, int vertexIndex)
{
    assert(faceIndex >= 0 && faceIndex < (int)faces.size());
    assert(vertexIndex >= 0 && vertexIndex < 3);
    int index = faces[faceIndex][vertexIndex*3 + 2];
    assert(index >= 0 && index < (int)vertexNormals.size());
    return vertexNormals[index];
}

TGAColor Model::getTextureColor(Vec2f uv)
{
    assert(uv.u >= 0.0 && uv.u <= 1.0);
    assert(uv.v >= 0.0 && uv.v <= 1.0);

    Vec2i texel(uv.u * diffuseMap.get_width(),
                uv.v * diffuseMap.get_height());

    return diffuseMap.get(texel.x, texel.y);
}

Vec3f Model::getTextureNormal(Vec2f uv)
{
    assert(uv.u >= 0.0 && uv.u <= 1.0);
    assert(uv.v >= 0.0 && uv.v <= 1.0);

    Vec2i texel(uv.u * normalMap.get_width(),
                uv.v * normalMap.get_height());
    TGAColor normalColor = normalMap.get(texel.u, texel.y);
    Vec3f vertexNormal((normalColor.r / 255.0f) * 2.0f - 1.0f,
                       (normalColor.g / 255.0f) * 2.0f - 1.0f,
                       (normalColor.b / 255.0f) * 2.0f - 1.0f);

    return vertexNormal;
}

Vec3f Model::getTangentNormal(Vec2f uv)
{
    assert(uv.u >= 0.0 && uv.u <= 1.0);
    assert(uv.v >= 0.0 && uv.v <= 1.0);

    Vec2i texel(uv.u * tangentMap.get_width(),
                uv.v * tangentMap.get_height());
    TGAColor tangentColor = tangentMap.get(texel.u, texel.y);
    Vec3f tangentNormal((tangentColor.r / 255.0f) * 2.0f - 1.0f,
                        (tangentColor.g / 255.0f) * 2.0f - 1.0f,
                        (tangentColor.b / 255.0f) * 2.0f - 1.0f);

    return tangentNormal;
}

float Model::getSpecularPower(Vec2f uv)
{
    assert(uv.u >= 0.0 && uv.u <= 1.0);
    assert(uv.v >= 0.0 && uv.v <= 1.0);

    Vec2i texel(uv.u * specularMap.get_width(),
                uv.v * specularMap.get_height());

    return specularMap.get(texel.x, texel.y).raw[0] / 1.0f;
}
