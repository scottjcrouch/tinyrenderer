#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>

#include "geometry.h"

class Model
{
public:
    Model(const char *filename);

    int numFaces();
    std::vector<int> getFace(int index);
    Vec3f getVertex(int index);
    Vec3f getTextureVertex(int index);
    Vec3f getVertexNormal(int index);

private:
    std::vector<Vec3f> vertices;
    std::vector<std::vector<int>> faces;
    std::vector<Vec3f> textureVertices;
    std::vector<Vec3f> vertexNormals;
};

#endif // __MODEL_H__
