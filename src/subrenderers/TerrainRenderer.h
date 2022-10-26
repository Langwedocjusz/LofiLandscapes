#pragma once

#include "Shader.h"

struct TerrainSettings{
    int N = 33;
    float L = 4.0f;
};

class TerrainRenderer {
public:
    TerrainRenderer();
    ~TerrainRenderer();

    void DisplaceVertices(float scale_xz, float scale_y,
                          float offset_x, float offset_z);
    void BindAndDraw();

private:
    unsigned int m_TerrainVAO, m_TerrainVBO, m_TerrainEBO;
    std::vector<float> m_TerrainVertexData;
    std::vector<unsigned int> m_TerrainIndexData;

    unsigned int m_TerrainVAO2, m_TerrainVBO2, m_TerrainEBO2;
    std::vector<float> m_TerrainVertexData2;
    std::vector<unsigned int> m_TerrainIndexData2;
    
    unsigned int m_TerrainVAO3, m_TerrainVBO3, m_TerrainEBO3;
    std::vector<float> m_TerrainVertexData3;
    std::vector<unsigned int> m_TerrainIndexData3;

    unsigned int m_TerrainVAO4, m_TerrainVBO4, m_TerrainEBO4;
    std::vector<float> m_TerrainVertexData4;
    std::vector<unsigned int> m_TerrainIndexData4;
    
    TerrainSettings m_Settings;

    Shader m_DisplaceShader;

    void GenerateGrid(std::vector<float>& vert, std::vector<unsigned int>& idx, 
        unsigned int N, float L, float global_offset_x, float global_offset_y, 
        unsigned int LodLevel); 
};
