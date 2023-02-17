#pragma once

#include "Shader.h"
#include "GLUtils.h"
#include "TextureEditor.h"

struct ScaleSettings{
    float ScaleXZ = 100.0f;
    float ScaleY = 20.0f;
};

struct AOSettings{
    int Samples = 16;
    float R = 0.01;
};

struct ShadowmapSettings{
    int MinLevel = 5;
    int StartCell = 32;
    int MipOffset = 0;

    float NudgeFac = 1.02f;

    bool Soft = true;
    float Sharpness = 1.0f;
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

    void Init(int height_res, int shadow_res, int wrap_type);
    void Update(const glm::vec3& sun_dir);

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
    TextureEditor m_HeightEditor;
    Shader m_NormalmapShader, m_ShadowmapShader;

    Shader m_MipShader;
    
    ScaleSettings     m_ScaleSettings;
    ShadowmapSettings m_ShadowSettings;
    AOSettings        m_AOSettings;

    MapUpdateFlags m_UpdateFlags = MapUpdateFlags::None;

    int m_MipLevels = 0;
    
    void UpdateHeight();
    void UpdateNormal();
    void UpdateShadow(const glm::vec3& sun_dir);

    void GenMaxMips();
};

bool operator==(const ScaleSettings& lhs, const ScaleSettings& rhs);
bool operator!=(const ScaleSettings& lhs, const ScaleSettings& rhs);

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);
bool operator!=(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);

bool operator==(const AOSettings& lhs, const AOSettings& rhs);
bool operator!=(const AOSettings& lhs, const AOSettings& rhs);

MapUpdateFlags operator|(MapUpdateFlags x, MapUpdateFlags y);
MapUpdateFlags operator&(MapUpdateFlags x, MapUpdateFlags y);
