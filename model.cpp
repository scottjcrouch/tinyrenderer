#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

#include "model.h"

Model::Model(const char *filename) : vertices(), faces()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    assert(!in.fail());

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
            float u, v;
            iss >> dummy_char >> dummy_char >> u >> v;
            textureVertices.emplace_back(u, v);
        } else if (!line.compare(0, 3, "vn ")) {
            float i, j, k;
            iss >> dummy_char >> dummy_char >> i >> j >> k;
            vertexNormals.emplace_back(i, j, k);
        }
    }
}

int Model::numFaces() { return faces.size(); }

std::vector<int> Model::getFace(int index) { return faces[index]; }

Vec3f Model::getVertex(int index) { return vertices[index]; }

Vec2f Model::getTextureVertex(int index) { return textureVertices[index]; }

Vec3f Model::getVertexNormal(int index) { return vertexNormals[index]; }
