#ifndef __GL_H__
#define __GL_H__

#include "tgaimage.h"
#include "geometry.h"

// Globals defined in gl.cpp
extern Matrix viewPort;
extern Matrix projection;
extern Matrix modelView;

// These modify the viewport, projection, and modelView matrices respectively,
// and are useful for implementing the vertex shader.
void view(int minX, int minY, int width, int height);
void project(float coeff=0.f); // coeff = -1/c
void lookAt(Vec3f eye, Vec3f center, Vec3f up);

// Shader operations which we provide to the triangle rasterizer.
struct IShader {
    virtual ~IShader() { }
    virtual Vec3f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(const Vec3f &baryCoords, TGAColor &color) = 0;
};

// The triangle rasterizer
void drawTriangle(const std::array<Vec3f, 3> &vertices,
                  IShader &shader,
                  TGAImage &image,
                  std::vector<float> &zBuffer);

#endif // __GL_H__
