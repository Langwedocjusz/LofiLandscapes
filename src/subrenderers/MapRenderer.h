#pragma once

#include "Shader.h"
#include "GLUtils.h"

struct HeightmapSettings{
    int Resolution = 4096;
    int Octaves = 10;
    float Offset[2] = {0.0f, 0.0f};
};

struct ScaleSettings{
    float ScaleXZ = 100.0f;
    float ScaleY = 20.0f;
};

struct AOSettings{
    int Samples = 16;
    float R = 0.01;
};

struct ShadowmapSettings{
    int Resolution = 4096;
    int Steps = 64;
    float MinT = 0.01f;
    float MaxT = 0.8f;
    float Bias = 6.0f;
};

enum class MapUpdateFlags {
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

    void ImGuiTerrain(bool &open, bool update_shadows);
    void ImGuiShadowmap(bool &open, bool update_shadows);
    void RequestShadowUpdate();

    bool GeometryShouldUpdate();

    ScaleSettings getScaleSettings() {return m_ScaleSettings;}

private:
    Texture m_Heightmap, m_Normalmap, m_Shadowmap; 
    Shader m_HeightmapShader, m_NormalmapShader, m_ShadowmapShader;
    
    HeightmapSettings m_HeightSettings;
    ScaleSettings     m_ScaleSettings;
    ShadowmapSettings m_ShadowSettings;
    AOSettings        m_AOSettings;

    MapUpdateFlags m_UpdateFlags = MapUpdateFlags::None;
    
    void UpdateHeight();
    void UpdateNormal();
    void UpdateShadow(float theta, float phi);    
};

bool operator==(const HeightmapSettings& lhs, const HeightmapSettings& rhs);
bool operator!=(const HeightmapSettings& lhs, const HeightmapSettings& rhs);

bool operator==(const ScaleSettings& lhs, const ScaleSettings& rhs);
bool operator!=(const ScaleSettings& lhs, const ScaleSettings& rhs);

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);
bool operator!=(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);

bool operator==(const AOSettings& lhs, const AOSettings& rhs);
bool operator!=(const AOSettings& lhs, const AOSettings& rhs);

MapUpdateFlags operator|(MapUpdateFlags x, MapUpdateFlags y);
MapUpdateFlags operator&(MapUpdateFlags x, MapUpdateFlags y);
