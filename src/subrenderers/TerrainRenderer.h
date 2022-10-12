#pragma once

#include "Shader.h"

class TerrainRenderer {
public:
    TerrainRenderer(unsigned int N, float L);
    ~TerrainRenderer();

    void DisplaceVertices();
    void BindGeometry();
    void Draw();

private:
    unsigned int m_TerrainVAO, m_TerrainVBO, m_TerrainEBO;
    std::vector<float> m_TerrainVertexData;
    std::vector<unsigned int> m_TerrainIndexData;

    unsigned int m_N;
    float m_L;

    Shader m_DisplaceShader;
};
