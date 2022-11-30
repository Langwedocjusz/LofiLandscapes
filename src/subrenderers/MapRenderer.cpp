#include "MapRenderer.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>

MapRenderer::MapRenderer()
    : m_NormalmapShader("res/shaders/terrain/normal.glsl"),
      m_ShadowmapShader("res/shaders/terrain/shadow.glsl")
{
    //-----Initialize Textures
    //-----Heightmap
    const int tex_res = 4096; //To-do: maybe add as constructor argument
    
    TextureSpec heightmap_spec = TextureSpec{
        tex_res, GL_R32F, GL_RGBA, GL_UNSIGNED_BYTE, 
        GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, 
        GL_CLAMP_TO_BORDER, 
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_Heightmap.Initialize(heightmap_spec);
    
    //Generate mips for heightmap:
    m_Heightmap.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    //-----Normal map: 
    TextureSpec normal_spec = TextureSpec{
        tex_res, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 
        GL_LINEAR, GL_LINEAR, 
        GL_CLAMP_TO_BORDER, 
        //Pointing up (0,1,0), after compression -> (0.5, 1.0, 0.5):
        {0.5f, 1.0f, 0.5f, 1.0f}
    };

    m_Normalmap.Initialize(normal_spec);

    //-----Shadow map
    const int shadow_res = m_ShadowSettings.Resolution;

    TextureSpec shadow_spec = TextureSpec{
        shadow_res, GL_R8, GL_RED, GL_UNSIGNED_BYTE, 
        GL_LINEAR, GL_LINEAR, 
        GL_CLAMP_TO_BORDER, 
        {1.0f, 1.0f, 1.0f, 1.0f}
    };

    m_Shadowmap.Initialize(shadow_spec);    

    //-----Setup heightmap editor:
    std::vector<std::string> labels{ "Average", "Add", "Subtract" };

    m_HeightEditor.RegisterShader("Const Value", "res/shaders/terrain/const_val.glsl");
    m_HeightEditor.AttachSliderFloat("Const Value", "uValue", "Value", 0.0, 1.0, 0.0);

    m_HeightEditor.RegisterShader("FBM", "res/shaders/terrain/fbm.glsl");
    m_HeightEditor.AttachConstInt("FBM", "uResolution", 4096);
    m_HeightEditor.AttachSliderInt("FBM", "uOctaves", "Octaves", 1, 16, 8);
    m_HeightEditor.AttachSliderFloat("FBM", "uScale", "Scale", 1.0, 64.0, 32.0);
    m_HeightEditor.AttachSliderFloat("FBM", "uRoughness", "Roughness", 0.0, 1.0, 0.5);
    m_HeightEditor.AttachGLEnum("FBM", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.AttachSliderFloat("FBM", "uWeight", "Weight", 0.0, 1.0, 1.0);

    m_HeightEditor.RegisterShader("Voronoi", "res/shaders/terrain/voronoi.glsl");
    m_HeightEditor.AttachConstInt("Voronoi", "uResolution", 4096);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uScale", "Scale", 1.0, 64.0, 8.0);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uRandomness", "Randomness", 0.0, 1.0, 1.0);

    std::vector<std::string> voro_types{ "F1", "F2", "F2_F1" };
    m_HeightEditor.AttachGLEnum("Voronoi", "uVoronoiType", "Type", voro_types);

    m_HeightEditor.AttachGLEnum("Voronoi", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uWeight", "Weight", 0.0, 1.0, 1.0);

    m_HeightEditor.RegisterShader("Radial cutoff", "res/shaders/terrain/radial_cutoff.glsl");
    m_HeightEditor.AttachConstInt("Radial cutoff", "uResolution", 4096);
    m_HeightEditor.AttachSliderFloat("Radial cutoff", "uBias", "Bias", 0.0, 1.0, 0.5);
    m_HeightEditor.AttachSliderFloat("Radial cutoff", "uSlope", "Slope", 0.0, 10.0, 4.0);

    //Initial procedures:
    m_HeightEditor.AddProcedureInstance("Const Value");
    m_HeightEditor.AddProcedureInstance("FBM");
    m_HeightEditor.AddProcedureInstance("Radial cutoff");

    //-----Set update flags
    m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Height;
    m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Normal;
    m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Shadow;
}

MapRenderer::~MapRenderer() {}

void MapRenderer::UpdateHeight() { 
    const int res = m_Heightmap.getSpec().Resolution;

    m_Heightmap.BindImage(0, 0);
    m_HeightEditor.OnDispatch(res);

    m_Heightmap.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);
}

void MapRenderer::UpdateNormal() {
    const int res = m_Normalmap.getSpec().Resolution;

    m_Heightmap.Bind();
    m_Normalmap.BindImage(0, 0);
 
    m_NormalmapShader.Bind();
    m_NormalmapShader.setUniform1i("uResolution", res);
    m_NormalmapShader.setUniform1f("uScaleXZ", m_ScaleSettings.ScaleXZ);
    m_NormalmapShader.setUniform1f("uScaleY" , m_ScaleSettings.ScaleY );

    m_NormalmapShader.setUniform1i("uAOSamples", m_AOSettings.Samples);
    m_NormalmapShader.setUniform1f("uAOR", m_AOSettings.R);

    glDispatchCompute(res/32, res/32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void MapRenderer::UpdateShadow(float theta, float phi) {
    const int res = m_Shadowmap.getSpec().Resolution;

    m_Heightmap.Bind();
    m_Shadowmap.BindImage(0, 0);
 
    m_ShadowmapShader.Bind();
    m_ShadowmapShader.setUniform1i("uResolution", res);
    m_ShadowmapShader.setUniform1f("uTheta", theta);
    m_ShadowmapShader.setUniform1f("uPhi", phi);
    m_ShadowmapShader.setUniform1f("uScaleXZ", m_ScaleSettings.ScaleXZ);
    m_ShadowmapShader.setUniform1f("uScaleY", m_ScaleSettings.ScaleY);
    m_ShadowmapShader.setUniform1f("uMinT", m_ShadowSettings.MinT);
    m_ShadowmapShader.setUniform1f("uMaxT", m_ShadowSettings.MaxT);
    m_ShadowmapShader.setUniform1i("uSteps", m_ShadowSettings.Steps);
    m_ShadowmapShader.setUniform1f("uBias", m_ShadowSettings.Bias);

    glDispatchCompute(res/32, res/32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void MapRenderer::Update(float theta, float phi) {
    if ((m_UpdateFlags & MapUpdateFlags::Height) != MapUpdateFlags::None)
        UpdateHeight();

    if ((m_UpdateFlags & MapUpdateFlags::Normal) != MapUpdateFlags::None)
        UpdateNormal();

    if ((m_UpdateFlags & MapUpdateFlags::Shadow) != MapUpdateFlags::None)
        UpdateShadow(theta, phi);

    m_UpdateFlags = MapUpdateFlags::None;
}

void MapRenderer::BindHeightmap(int id) {
    m_Heightmap.Bind(id);
}

void MapRenderer::BindNormalmap(int id) {
    m_Normalmap.Bind(id);
}

void MapRenderer::BindShadowmap(int id) {
    m_Shadowmap.Bind(id);
}

void MapRenderer::ImGuiTerrain(bool &open, bool update_shadows) {
    ScaleSettings temp_s = m_ScaleSettings;

    ImGui::Begin("Terrain editor", &open);

    ImGui::Text("Scale:");
    ImGuiUtils::SliderFloat("Scale xz", &(temp_s.ScaleXZ), 0.0f, 400.0f);
    ImGuiUtils::SliderFloat("Scale y" , &(temp_s.ScaleY ), 0.0f, 100.0f);
    ImGui::Separator();

    bool scale_changed = (temp_s != m_ScaleSettings);

    ImGui::Text("Heightmap procedures:");

    bool height_changed = m_HeightEditor.OnImGui();

    if (ImGuiUtils::Button("Add heightmap procedure")) {
        ImGui::OpenPopup("Choose procedure (heightmap)");
    }

    if (ImGui::BeginPopupModal("Choose procedure (heightmap)")) {

        if (ImGui::Button("Const Value")) {
            ImGui::CloseCurrentPopup();
            m_HeightEditor.AddProcedureInstance("Const Value");

            height_changed = true;
        }

        if (ImGui::Button("FBM")) {
            ImGui::CloseCurrentPopup();
            m_HeightEditor.AddProcedureInstance("FBM");

            height_changed = true;
        }

        if (ImGui::Button("Voronoi")) {
            ImGui::CloseCurrentPopup();
            m_HeightEditor.AddProcedureInstance("Voronoi");

            height_changed = true;
        }

        if (ImGui::Button("Radial Cutoff")) {
            ImGui::CloseCurrentPopup();
            m_HeightEditor.AddProcedureInstance("Radial cutoff");

            height_changed = true;
        }

        ImGui::EndPopup();
    }

    ImGui::End();

    if (height_changed) {
        m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Height;
        m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Normal;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Shadow;
    }

    else if (scale_changed) {
        m_ScaleSettings = temp_s;
        m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Normal;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Shadow;
    }

}

void MapRenderer::ImGuiShadowmap(bool &open, bool update_shadows) {
    ShadowmapSettings temp = m_ShadowSettings;
    AOSettings temp2 = m_AOSettings;

    ImGui::Begin("Shadow/AO settings", &open);

    ImGui::Text("Shdowmap Settings:");
    ImGui::Spacing();

    ImGuiUtils::SliderFloat("Min t", &temp.MinT, 0.0, 0.5);
    ImGuiUtils::SliderFloat("Max t", &temp.MaxT, 0.0, 1.0);
    ImGuiUtils::SliderFloat("Mip bias", &temp.Bias, 0.0, 20.0);
    ImGuiUtils::SliderInt("Steps", &temp.Steps, 1, 64);

    ImGui::Separator();
    ImGui::Text("AO Settings:");
    ImGui::Spacing();

    ImGuiUtils::SliderInt("AO Samples", &temp2.Samples, 1, 64);
    ImGuiUtils::SliderFloat("AO Radius", &temp2.R, 0.0, 0.1);

    ImGui::End();

    if (temp2 != m_AOSettings) {
        m_AOSettings = temp2;
        m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Normal;
    }

    if (temp != m_ShadowSettings) {
        m_ShadowSettings = temp;
        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Shadow;
    }
}

void MapRenderer::RequestShadowUpdate() {
    m_UpdateFlags = m_UpdateFlags | MapUpdateFlags::Shadow;
}

bool MapRenderer::GeometryShouldUpdate() {
    return (m_UpdateFlags & MapUpdateFlags::Height) != MapUpdateFlags::None ||
           (m_UpdateFlags & MapUpdateFlags::Normal) != MapUpdateFlags::None;
}

//Settings structs operator overloads:

bool operator==(const ScaleSettings& lhs, const ScaleSettings& rhs) {
    return (lhs.ScaleXZ == rhs.ScaleXZ) && (lhs.ScaleY == rhs.ScaleY);
}

bool operator!=(const ScaleSettings& lhs, const ScaleSettings& rhs) {
    return !(lhs==rhs);
}

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs) {
    return (lhs.Steps == rhs.Steps) && (lhs.MinT == rhs.MinT)
        && (lhs.MaxT == rhs.MaxT) && (lhs.Bias == rhs.Bias);
}

bool operator!=(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs) {
    return !(lhs==rhs);
}

bool operator==(const AOSettings& lhs, const AOSettings& rhs) {
    return (lhs.Samples == rhs.Samples) && (lhs.R == rhs.R);
}

bool operator!=(const AOSettings& lhs, const AOSettings& rhs) {
    return !(lhs==rhs);
}

MapUpdateFlags operator|(MapUpdateFlags x, MapUpdateFlags y) {
    return static_cast<MapUpdateFlags>(static_cast<int>(x) 
                                     | static_cast<int>(y));
}

MapUpdateFlags operator&(MapUpdateFlags x, MapUpdateFlags y) {
    return static_cast<MapUpdateFlags>(static_cast<int>(x) 
                                     & static_cast<int>(y));
}
