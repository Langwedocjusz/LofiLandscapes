#pragma once

#include <vector>

class Scene{
public:
    Scene(int N, float L);
    ~Scene();

    void BindTerrain();
    void DrawTerrain();
    void BindQuad();
    void DrawQuad();

    unsigned int getTerrainVAO() {return m_TerrainVAO;}
    unsigned int getTerrainVBO() {return m_TerrainVBO;}
    unsigned int getTerrainVertCount() {return m_TerrainVertexData.size();}

private:
    int m_N;

    unsigned int m_TerrainVAO, m_TerrainVBO, m_TerrainEBO;
    std::vector<float> m_TerrainVertexData;
    std::vector<unsigned int> m_TerrainIndexData;

    unsigned int m_QuadVAO, m_QuadVBO, m_QuadEBO;

    float m_QuadVertexData[12] = {-1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f,
                                   1.0f,-1.0f, 1.0f,
                                  -1.0f,-1.0f, 1.0f };
    unsigned int m_QuadIndexData[6] = {0,1,3, 1,2,3};
};
