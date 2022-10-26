#pragma once

#include "Shader.h"

struct HeightmapSettings{
    int Resolution = 4096;
    int Octaves = 10;
    float Offset[2] = {0.0f, 0.0f};
};

struct ScaleSettings{
    float ScaleXZ = 100.0f;
    float ScaleY = 20.0f;
};

struct ShadowmapSettings{
    int Resolution = 4096;
    int Steps = 32;
    float MinT = 0.01f;
    float MaxT = 1.0f;
    float Bias = 0.0f;
};

enum class UpdateFlags {
    None   =      0,
    Height = (1<<0),
    Normal = (1<<1),
    Shadow = (1<<2)
};

class MapRenderer {
public:
    MapRenderer();
    ~MapRenderer();

    void Update(float theta, float phi);

    void BindHeightmap(int id=0);
    void BindNormalmap(int id=0);
    void BindShadowmap(int id=0);

    void ImGuiTerrain(bool update_shadows);
    void ImGuiShadowmap(bool update_shadows);
    void RequestShadowUpdate();

    bool GeometryShouldUpdate();

    ScaleSettings getScaleSettings() {return m_ScaleSettings;}

private:
    unsigned int m_QuadVAO, m_QuadVBO, m_QuadEBO;
    unsigned int m_HeightmapFBO, m_NormalmapFBO, m_ShadowFBO;
    unsigned int m_HeightmapTexture, m_NormalmapTexture, m_ShadowTexture;
    
    Shader m_HeightmapShader, m_NormalmapShader, m_ShadowmapShader;
    
    HeightmapSettings m_HeightSettings;
    ScaleSettings     m_ScaleSettings;
    ShadowmapSettings m_ShadowSettings;

    UpdateFlags m_UpdateFlags = UpdateFlags::None;

    float m_QuadVertexData[12] = {-1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f,
                                   1.0f,-1.0f, 1.0f,
                                  -1.0f,-1.0f, 1.0f };
    
    unsigned int m_QuadIndexData[6] = {0,1,3, 1,2,3};
    
    void UpdateHeight();
    void UpdateNormal();
    void UpdateShadow(float theta, float phi);
    
    void BindGeometry();
    void Draw();
};

bool operator==(const HeightmapSettings& lhs, const HeightmapSettings& rhs);
bool operator!=(const HeightmapSettings& lhs, const HeightmapSettings& rhs);

bool operator==(const ScaleSettings& lhs, const ScaleSettings& rhs);
bool operator!=(const ScaleSettings& lhs, const ScaleSettings& rhs);

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);
bool operator!=(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);

UpdateFlags operator|(UpdateFlags x, UpdateFlags y);
UpdateFlags operator&(UpdateFlags x, UpdateFlags y);
