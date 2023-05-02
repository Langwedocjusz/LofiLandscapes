#include "MaterialGenerator.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>

MaterialGenerator::MaterialGenerator()
    : m_NormalShader("res/shaders/materials/normal.glsl")
    , m_HeightEditor("Height", m_Layers)
    , m_AlbedoEditor("Albedo", m_Layers)
    , m_RoughnessEditor("Roughness", m_Layers)
{
    //=====Initialize the textures:

    m_Height.Initialize(TextureSpec{
        512, 512, GL_R16F, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    }, m_Layers);

    m_Normal.Initialize(TextureSpec{
        512, 512, GL_RGBA8, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        GL_REPEAT,
        {0.5f, 1.0f, 0.5f, 1.0f}
    }, m_Layers);

    m_Normal.Bind();
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    m_Albedo.Initialize(TextureSpec{
        512, 512, GL_RGBA8, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.7f}
    }, m_Layers);

    m_Albedo.Bind();
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    //=====Initialize material editors:
    std::vector<std::string> labels{ "Average", "Add", "Subtract" };
    
    //Heightmap
    m_HeightEditor.RegisterShader("Const Value", "res/shaders/materials/const_val.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Const Value", "uValue", "Value", 0.0, 1.0, 0.0);

    m_HeightEditor.RegisterShader("FBM", "res/shaders/materials/fbm.glsl");
    m_HeightEditor.Attach<ConstIntTask>("FBM", "uResolution", m_Height.getSpec().ResolutionX);
    m_HeightEditor.Attach<SliderIntTask>("FBM", "uOctaves", "Octaves", 1, 16, 8);
    m_HeightEditor.Attach<SliderIntTask>("FBM", "uScale", "Scale", 0, 100, 1);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uRoughness", "Roughness", 0.0, 1.0, 0.5);
    m_HeightEditor.Attach<GLEnumTask>("FBM", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uWeight", "Weight", 0.0, 1.0, 1.0);


    m_HeightEditor.RegisterShader("Voronoi", "res/shaders/materials/voronoi.glsl");
    m_HeightEditor.Attach<ConstIntTask>("Voronoi", "uResolution", m_Height.getSpec().ResolutionX);
    m_HeightEditor.Attach<SliderIntTask>("Voronoi", "uScale", "Scale", 0, 100, 1);
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uRandomness", "Randomness", 0.0, 1.0, 1.0);
    
    std::vector<std::string> voro_types{"F1", "F2", "F2_F1"};
    m_HeightEditor.Attach<GLEnumTask>("Voronoi", "uVoronoiType", "Type", voro_types);
    
    m_HeightEditor.Attach<GLEnumTask>("Voronoi", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uWeight", "Weight", 0.0, 1.0, 1.0);

    //Albedo
    m_AlbedoEditor.RegisterShader("Const Albedo", "res/shaders/materials/const_albedo.glsl");
    m_AlbedoEditor.Attach<ColorEdit3Task>("Const Albedo", "uCol", "Albedo", glm::vec3(0.005f));

    m_AlbedoEditor.RegisterShader("Albedo Ramp", "res/shaders/materials/albedo_ramp.glsl");
    m_AlbedoEditor.Attach<ConstIntTask>("Albedo Ramp", "uResolution", m_Albedo.getSpec().ResolutionX);
    m_AlbedoEditor.Attach<SliderFloatTask>("Albedo Ramp", "uEdge1", "Edge 1", 0.0f, 1.0f, 0.0f);
    m_AlbedoEditor.Attach<SliderFloatTask>("Albedo Ramp", "uEdge2", "Edge 2", 0.0f, 1.0f, 1.0f);
    m_AlbedoEditor.Attach<ColorEdit3Task>("Albedo Ramp", "uCol1", "Albedo 1", glm::vec3(0.0f));
    m_AlbedoEditor.Attach<ColorEdit3Task>("Albedo Ramp", "uCol2", "Albedo 2", glm::vec3(1.0f));

    //Roughness
    m_RoughnessEditor.RegisterShader("Const Roughness", "res/shaders/materials/const_val_roughness.glsl");
    m_RoughnessEditor.Attach<SliderFloatTask>("Const Roughness", "uValue", "Roughness", 0.003, 1.0, 0.7);

    m_RoughnessEditor.RegisterShader("Roughness Ramp", "res/shaders/materials/roughness_ramp.glsl");
    m_RoughnessEditor.Attach<ConstIntTask>("Roughness Ramp", "uResolution", m_Albedo.getSpec().ResolutionX);
    m_RoughnessEditor.Attach<SliderFloatTask>("Roughness Ramp", "uEdge1", "Edge 1", 0.0f, 1.0f, 0.0f);
    m_RoughnessEditor.Attach<SliderFloatTask>("Roughness Ramp", "uEdge2", "Edge 2", 0.0f, 1.0f, 1.0f);
    m_RoughnessEditor.Attach<SliderFloatTask>("Roughness Ramp", "uVal1", "Value 1", 0.003f, 1.0f, 0.003f);
    m_RoughnessEditor.Attach<SliderFloatTask>("Roughness Ramp", "uVal2", "Value 2", 0.003f, 1.0f, 1.0f);

    //Initial procedures
    for (int i = 0; i < m_Layers; i++) {
        m_HeightEditor.AddProcedureInstance(i, "Const Value");
        m_AlbedoEditor.AddProcedureInstance(i, "Const Albedo");
        m_RoughnessEditor.AddProcedureInstance(i, "Const Roughness");
    }

    //Update all layers
    for (int i = 0; i < m_Layers; i++)
    {
        m_UpdateFlags = Height | Normal | Albedo;
        m_Current = i;
        Update();
    }

    m_Current = 0;
}

MaterialGenerator::~MaterialGenerator() {

}

void MaterialGenerator::Update() { 

    //Draw to heightmap:
    if ((m_UpdateFlags & Height) != None)
    {
        const int res = m_Height.getSpec().ResolutionX;
        
        m_Height.BindImage(0, m_Current, 0);
        m_HeightEditor.OnDispatch(m_Current, res);
    }

    //Draw to normal:
    if ((m_UpdateFlags & Normal) != None)
    {
        const int res = m_Normal.getSpec().ResolutionX;
        m_Height.BindLayer(0, m_Current);
        
        m_Normal.BindImage(0, m_Current, 0);

        m_NormalShader.Bind();
        m_NormalShader.setUniform1i("uResolution", res);
        m_NormalShader.setUniform1f("uAOStrength", 1.0f/m_AOStrength);
        m_NormalShader.setUniform1f("uAOSpread", m_AOSpread);
        m_NormalShader.setUniform1f("uAOContrast", m_AOContrast);

        //Magic number "32" needs to be the same as local size
        //declared in the compute shader files
        glDispatchCompute(res/32, res/32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    
        m_Normal.Bind();
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    //Draw to albedo/roughness:
    if ((m_UpdateFlags & Albedo) != None)
    {
        const int res = m_Albedo.getSpec().ResolutionX;
        m_Height.BindLayer(0, m_Current);

        m_Albedo.BindImage(0, m_Current, 0);
        m_AlbedoEditor.OnDispatch(m_Current, res);
        m_RoughnessEditor.OnDispatch(m_Current, res);
    
        m_Albedo.Bind();
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    m_UpdateFlags = None;
}

void MaterialGenerator::OnImGui(bool& open) {

    ImGui::Begin("Material editor", &open, ImGuiWindowFlags_NoFocusOnAppearing);
    
    ImGui::Columns(2, "###col");
    ImGuiUtils::SliderInt("Currently editing", &m_Current, 0, m_Layers-1);
    ImGui::Columns(1, "###col");

    ImGui::Text("Heightmap procedures:");

    if (m_HeightEditor.OnImGui(m_Current)) {
        m_UpdateFlags = m_UpdateFlags | Height;
        m_UpdateFlags = m_UpdateFlags | Normal;
        m_UpdateFlags = m_UpdateFlags | Albedo;
    }

    ImGuiUtils::Separator();

    ImGui::Text("Normal/AO map settings:");

    float tmp_str = m_AOStrength, tmp_spr = m_AOSpread, tmp_c = m_AOContrast;  

    ImGui::Columns(2, "###col");
    ImGuiUtils::SliderFloat("AO Strength", &tmp_str, 0.01, 1.0);
    ImGuiUtils::SliderFloat("AO Spread"  , &tmp_spr, 1.00, 10.0);
    ImGuiUtils::SliderFloat("AO Contrast", &tmp_c,   0.10, 5.0);
    ImGui::Columns(1, "###col");

    if (tmp_str != m_AOStrength || tmp_spr != m_AOSpread || tmp_c != m_AOContrast) {
        m_AOStrength = tmp_str;
        m_AOSpread   = tmp_spr;
        m_AOContrast = tmp_c;

        m_UpdateFlags = m_UpdateFlags | Normal;
    }

    ImGuiUtils::Separator();

    ImGui::Text("Albedo procedures:");
    
    if (m_AlbedoEditor.OnImGui(m_Current)) {
        m_UpdateFlags = m_UpdateFlags | Albedo;
    }

    ImGuiUtils::Separator();

    ImGui::Text("Roughness procedures:");

    if (m_RoughnessEditor.OnImGui(m_Current)) {
        m_UpdateFlags = m_UpdateFlags | Albedo;
    }

    ImGui::End();

    Update();
}

void MaterialGenerator::BindAlbedo(int id) const {
    m_Albedo.Bind(id);
}

void MaterialGenerator::BindNormal(int id) const {
    m_Normal.Bind(id);
}
