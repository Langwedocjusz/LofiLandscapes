#pragma once

#include "GLUtils.h"
#include "MaterialEditor.h"


enum class MaterialUpdateFlags {
    None   = (   0),
    Height = (1<<0),
    Normal = (1<<1),
    Albedo = (1<<2)
};

class MaterialRenderer{
public:
    MaterialRenderer();
    ~MaterialRenderer();

    void Update();
    void OnImGui();

    void BindAlbedo(int id=0);
    void BindNormal(int id=0);

private:
    Texture m_Height, m_Normal, m_Albedo;

    //Heightmap generation
    MaterialEditor m_HeightEditor;
    //Normalmap generation
    Shader m_NormalShader;
    float m_AOStrength = 1.0f, m_AOSpread = 1.0f, m_AOContrast = 1.0f;
    //Albedo generation:
    MaterialEditor m_AlbedoEditor;

    MaterialUpdateFlags m_UpdateFlags = MaterialUpdateFlags::None;
};

MaterialUpdateFlags operator|(MaterialUpdateFlags x, MaterialUpdateFlags y);
MaterialUpdateFlags operator&(MaterialUpdateFlags x, MaterialUpdateFlags y);
