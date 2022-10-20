#pragma once

#include "Shader.h"

struct HeightmapSettings{
    int Resolution = 4096;
    int Octaves = 10;
    float ScaleXZ = 100.0f;
    float ScaleY = 20.0f;
    float Offset[2] = {0.0f, 0.0f};
};

bool operator==(const HeightmapSettings& lhs, const HeightmapSettings& rhs);
bool operator!=(const HeightmapSettings& lhs, const HeightmapSettings& rhs);

class MapRenderer {
public:
    MapRenderer();
    ~MapRenderer();

    void BindGeometry();
    void Draw();

    void UpdateHeight();
    void UpdateNormal();
    void UpdateShadow(float theta, float phi);

    void BindHeightmap(int id=0);
    void BindNormalmap(int id=0);
    void BindShadowmap(int id=0);

    void setSettings(const HeightmapSettings& we) 
    {m_Settings = we;}
    const HeightmapSettings& getSettings()
    {return m_Settings;}

private:
    unsigned int m_QuadVAO, m_QuadVBO, m_QuadEBO;
    unsigned int m_HeightmapFBO, m_HeightmapTexture;
    unsigned int m_NormalmapFBO, m_NormalmapTexture;
    unsigned int m_ShadowFBO, m_ShadowTexture;

    float m_QuadVertexData[12] = {-1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f,
                                   1.0f,-1.0f, 1.0f,
                                  -1.0f,-1.0f, 1.0f };
    unsigned int m_QuadIndexData[6] = {0,1,3, 1,2,3};

    Shader m_HeightmapShader, m_NormalmapShader, m_ShadowShader;
    HeightmapSettings m_Settings;
};
