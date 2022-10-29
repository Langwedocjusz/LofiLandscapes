#pragma once

#include "Shader.h"

class Drawable{
public:
    Drawable();
    ~Drawable();

    void GenBuffers();
    void DispatchCompute();
    void Draw();

    unsigned int VAO, VBO, EBO;
    unsigned int ElementCount;
    std::vector<float> VertexData;
    std::vector<unsigned int> IndexData;
};

class ClipmapRing {
public:
    ClipmapRing(int N, float L, unsigned int level);
    ~ClipmapRing();

    void DispatchCompute();
    void Draw();
private:
    Drawable m_Grid, m_FillX, m_FillY;

    void GenerateGrid(unsigned int N, float L, float global_offset_x, 
                      float global_offset_y, unsigned int LodLevel);
    void GenerateFill(unsigned int N, float L, unsigned int LodLevel);
};

class ClipmapRenderer {
public:
    ClipmapRenderer();
    ~ClipmapRenderer();

    void DisplaceVertices(float scale_xz, float scale_y,
                          float offset_x, float offset_z);
    void BindAndDraw();
private:
    int m_N = 33;
    float m_L = 4.0f;

    ClipmapRing m_Lod0, m_Lod1, m_Lod2, m_Lod3, m_Lod4;
    Shader m_DisplaceShader;
};
