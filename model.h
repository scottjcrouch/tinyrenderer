#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>

#include "geometry.h"

class Model
{
public:
    Model(const char *filename);
    ~Model();
    int numVerts();
    int numFaces();
    Vec3f vert(int i);
    std::vector<int> face(int idx);

private:
    std::vector<Vec3f> verts;
    std::vector<std::vector<int>> faces;
};

#endif // __MODEL_H__
