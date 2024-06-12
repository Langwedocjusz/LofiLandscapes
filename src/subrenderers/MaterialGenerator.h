#pragma once

#include "Texture.h"
#include "TextureEditor.h"
#include "ResourceManager.h"

class MaterialGenerator{
public:
    MaterialGenerator(ResourceManager& manager);

    void Init(int material_res);
    void Update();
    void OnImGui(bool& open);
    void OnSerialize(nlohmann::ordered_json& output);
    void OnDeserialize(nlohmann::ordered_json& input);

    void BindAlbedo(int id=0) const;
    void BindNormal(int id=0) const;

private:

    enum MaterialUpdateFlags {
        None   =  0,
        Height = (1 << 0),
        Normal = (1 << 1),
        Albedo = (1 << 2)
    };

    int m_UpdateFlags = None;

    const int m_Layers = 5;
    int m_Current = 0;

    ResourceManager& m_ResourceManager;

    //Heightmap generation
    TextureArrayEditor m_HeightEditor;
    //Normalmap generation
    std::shared_ptr<ComputeShader> m_NormalShader;

    float m_AOStrength = 1.0f, m_AOSpread = 1.0f, m_AOContrast = 1.0f;
    //Albedo generation:
    TextureArrayEditor m_AlbedoEditor;
    //Roughness generation:
    TextureArrayEditor m_RoughnessEditor;

    std::shared_ptr<TextureArray> m_Height, m_Normal, m_Albedo;
};
