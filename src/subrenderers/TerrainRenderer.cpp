#include "TerrainRenderer.h"

#include "imgui.h"
#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include "Profiler.h"

#include "glad/glad.h"

TerrainRenderer::TerrainRenderer(ResourceManager& manager)
    : m_ResourceManager(manager)
{
    m_ShadedShader    = m_ResourceManager.RequestVertFragShader("res/shaders/shaded.vert", "res/shaders/shaded.frag");
    m_WireframeShader = m_ResourceManager.RequestVertFragShader("res/shaders/wireframe.vert", "res/shaders/wireframe.frag");
}

TerrainRenderer::~TerrainRenderer() {}

void TerrainRenderer::RenderWireframe(const glm::mat4& mvp, const Camera& cam, const MapGenerator& map, const Clipmap& clipmap) {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    {
        ProfilerGPUEvent we("Terrain::PrepareWireframe");

        m_WireframeShader->Bind();
        m_WireframeShader->setUniform1f("uL", map.getScaleSettings().ScaleXZ);
        m_WireframeShader->setUniform2f("uPos", cam.getPos().x, cam.getPos().z);
        m_WireframeShader->setUniformMatrix4fv("uMVP", mvp);
    }
    
    {
        ProfilerGPUEvent we("Terrain::Draw");

        auto scale_y = map.getScaleSettings().ScaleY;
        clipmap.BindAndDraw(cam, scale_y);
    }
}

void TerrainRenderer::RenderShaded(const glm::mat4& mvp, const Camera& cam, const MapGenerator& map,
                                    const MaterialGenerator& material, const SkyRenderer& sky, const Clipmap& clipmap)
{
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        ProfilerGPUEvent we("Terrain::PrepareShaded");

        const float scale_xz = map.getScaleSettings().ScaleXZ;

        m_ShadedShader->Bind();
        m_ShadedShader->setUniform1f("uL", scale_xz);
        m_ShadedShader->setUniform3f("uLightDir", sky.getSunDir());
        m_ShadedShader->setUniform3f("uPos", cam.getPos());
        m_ShadedShader->setUniformMatrix4fv("uMVP", mvp);
        m_ShadedShader->setUniform1i("uShadow", int(m_Shadows));
        m_ShadedShader->setUniform1i("uMaterial", int(m_Materials));
        m_ShadedShader->setUniform1i("uFixTiling", int(m_FixTiling));
        m_ShadedShader->setUniform1i("uFog", int(m_Fog));
        m_ShadedShader->setUniform3f("uSunCol", m_SunCol);
        m_ShadedShader->setUniform1f("uSunStr", m_SunStr);
        m_ShadedShader->setUniform1f("uSkyDiff", m_SkyDiff);
        m_ShadedShader->setUniform1f("uSkySpec", m_SkySpec);
        m_ShadedShader->setUniform1f("uRefStr", m_RefStr);
        m_ShadedShader->setUniform1f("uTilingFactor", m_TilingFactor);
        m_ShadedShader->setUniform1f("uNormalStrength", m_NormalStrength);

        map.BindNormalmap(0);
        m_ShadedShader->setUniform1i("normalmap", 0);
        map.BindShadowmap(1);
        m_ShadedShader->setUniform1i("shadowmap", 1);
        map.BindMaterialmap(2);
        m_ShadedShader->setUniform1i("materialmap", 2);

        material.BindAlbedo(3);
        m_ShadedShader->setUniform1i("albedo", 3);
        material.BindNormal(4);
        m_ShadedShader->setUniform1i("normal", 4);

        sky.BindIrradiance(5);
        m_ShadedShader->setUniform1i("irradiance", 5);
        sky.BindPrefiltered(6);
        m_ShadedShader->setUniform1i("prefiltered", 6);
        sky.BindAerial(7);
        m_ShadedShader->setUniform1i("aerial", 7);
    }
    
    {
        ProfilerGPUEvent we("Terrain::Draw");

        auto scale_y = map.getScaleSettings().ScaleY;
        clipmap.BindAndDraw(cam, scale_y);
    }
}

void TerrainRenderer::OnImGui(bool& open) {
    ImGui::Begin(LOFI_ICONS_LIGHTING "Lighting", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    ImGuiUtils::BeginGroupPanel("Lighting options:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColColorEdit3("Sun Color", &m_SunCol);
    ImGuiUtils::ColSliderFloat("Sun", &m_SunStr, 0.0, 4.0);
    ImGuiUtils::ColSliderFloat("Sky Diffuse", &m_SkyDiff, 0.0, 1.0);
    ImGuiUtils::ColSliderFloat("Sky Specular", &m_SkySpec, 0.0, 1.0);
    ImGuiUtils::ColSliderFloat("Reflected", &m_RefStr, 0.0, 1.0);
    ImGuiUtils::ColCheckbox("Shadows", &m_Shadows);
    ImGuiUtils::ColCheckbox("Materials", &m_Materials);
    ImGuiUtils::ColCheckbox("Render fog", &m_Fog);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGuiUtils::BeginGroupPanel("Material options:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColCheckbox("Fix Tiling", &m_FixTiling);
    ImGuiUtils::ColSliderFloat("Tiling Factor", &m_TilingFactor, 0.0, 128.0);
    ImGuiUtils::ColSliderFloat("Normal Strength", &m_NormalStrength, 0.0, 1.0);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGuiUtils::BeginGroupPanel("Background:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColColorEdit3("ClearColor", m_ClearColor);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGui::End();
}