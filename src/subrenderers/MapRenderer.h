#pragma once

#include "Shader.h"

struct HeightmapParams{
    int Octaves = 10;
    int Resolution = 4096;
};

bool operator==(const HeightmapParams& lhs, const HeightmapParams& rhs);

class MapRenderer {
public:
    MapRenderer();
    ~MapRenderer();

    void BindGeometry();
    void Draw();
    void Update();

    void BindHeightmap();
    void BindNormalmap();

    void setHeightmapParams(const HeightmapParams& we) 
    {m_HeightmapParams = we;}
    HeightmapParams getHeightmapParams()
    {return m_HeightmapParams;}

private:
    unsigned int m_QuadVAO, m_QuadVBO, m_QuadEBO;
    unsigned int m_HeightmapFBO, m_HeightmapTexture;
    unsigned int m_NormalmapFBO, m_NormalmapTexture;

    float m_QuadVertexData[12] = {-1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f,
                                   1.0f,-1.0f, 1.0f,
                                  -1.0f,-1.0f, 1.0f };
    unsigned int m_QuadIndexData[6] = {0,1,3, 1,2,3};

    Shader m_HeightmapShader, m_NormalmapShader;
    HeightmapParams m_HeightmapParams;
};
