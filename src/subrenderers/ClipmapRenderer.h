#pragma once

#include "Shader.h"

class Drawable{
public:
    Drawable();
    ~Drawable();

    void GenBuffers();
    void DispatchCompute();
    void Draw();

    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int ElementCount = 0;
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

    void Init(int subdivisions, int levels);
    void DisplaceVertices(float scale_xz, float scale_y,
                          float offset_x, float offset_z);
    void BindAndDraw();
private:
    float m_L = 4.0f;

    std::vector<ClipmapRing> m_LodLevels;
    Shader m_DisplaceShader;
};
