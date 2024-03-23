#pragma once

#include "Shader.h"
#include "Texture.h"
#include "TextureEditor.h"
#include "ResourceManager.h"

#include "nlohmann/json.hpp"

struct AOSettings{
    int Samples = 16;
    float R = 0.005f;
};

struct ShadowmapSettings{
    int MinLevel = 5;
    int StartCell = 32;
    int MipOffset = 0;

    float NudgeFac = 1.02f;

    bool Soft = true;
    float Sharpness = 1.0f;
};

class MapGenerator {
public:
    MapGenerator(ResourceManager& manager);

    void Init(int height_res, int shadow_res, int wrap_type);
    void Update(const glm::vec3& sun_dir);

    void BindHeightmap(int id=0) const;
    void BindNormalmap(int id=0) const;
    void BindShadowmap(int id=0) const;

    void RequestShadowUpdate() const;

    void ImGuiTerrain(bool &open, bool update_shadows);
    void ImGuiShadowmap(bool &open, bool update_shadows);

    bool GeometryShouldUpdate();

    float getScaleXZ() const {return m_ScaleXZ;}
    float getScaleY() const {return m_ScaleY;}

    void OnSerialize(nlohmann::ordered_json& output);
    void OnDeserialize(nlohmann::ordered_json& input);

private:
    void UpdateHeight();
    void UpdateNormal();
    void UpdateShadow(const glm::vec3& sun_dir);

    void GenMaxMips();

    enum UpdateFlags {
        None     =  0,
        Height   = (1 << 0),
        Normal   = (1 << 1),
        Shadow   = (1 << 2),
    };

    TextureEditor m_HeightEditor;
    std::shared_ptr<Texture2D> m_Heightmap, m_Normalmap, m_Shadowmap;

    std::shared_ptr<ComputeShader> m_NormalmapShader, m_ShadowmapShader;
    std::shared_ptr<ComputeShader> m_MipShader;
    
    float m_ScaleXZ = 100.0f;
    float m_ScaleY = 20.0f;

    ShadowmapSettings m_ShadowSettings;
    AOSettings        m_AOSettings;

    mutable int m_UpdateFlags = None;
    int m_MipLevels = 0;
    
    ResourceManager& m_ResourceManager;
};

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);
bool operator!=(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs);

bool operator==(const AOSettings& lhs, const AOSettings& rhs);
bool operator!=(const AOSettings& lhs, const AOSettings& rhs);
