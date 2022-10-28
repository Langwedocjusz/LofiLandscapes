#pragma once

#include "Shader.h"

class ClipmapRing {
public:
    ClipmapRing(int N, float L, unsigned int level);
    ~ClipmapRing();

    void DispatchCompute();
    void Draw();
private:
    unsigned int m_VAO, m_VBO, m_EBO;
    unsigned int m_ElementCount;
    std::vector<float> m_VertexData;
    std::vector<unsigned int> m_IndexData;
    
    void GenerateGrid(unsigned int N, float L, float global_offset_x, 
            float global_offset_y, unsigned int LodLevel); 
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

    ClipmapRing m_Lod0, m_Lod1, m_Lod2, m_Lod3;
    Shader m_DisplaceShader;
};
