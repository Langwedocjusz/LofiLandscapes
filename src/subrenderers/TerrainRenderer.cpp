#include "TerrainRenderer.h"

#include "imgui.h"
#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include "Profiler.h"

#include "glad/glad.h"

TerrainRenderer::TerrainRenderer(ResourceManager& manager, const PerspectiveCamera& cam,
                                 const MapGenerator& map, const MaterialGenerator& material,
                                 const Clipmap& clipmap, const SkyRenderer& sky)
    : m_ResourceManager(manager)
    , m_Camera(cam)
    , m_Map(map)
    , m_Material(material)
    , m_Clipmap(clipmap)
    , m_Sky(sky)
{
    m_ShadedShader    = m_ResourceManager.RequestVertFragShader("res/shaders/shaded.vert", "res/shaders/shaded.frag");
    m_WireframeShader = m_ResourceManager.RequestVertFragShader("res/shaders/wireframe.vert", "res/shaders/wireframe.frag");
}

TerrainRenderer::~TerrainRenderer() {}

void TerrainRenderer::RenderWireframe() {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    {
        ProfilerGPUEvent we("Terrain::PrepareWireframe");

        const glm::mat4 mvp = m_Camera.getViewProjMatrix();

        m_WireframeShader->Bind();
        m_WireframeShader->setUniform1f("uL", m_Map.getScaleSettings().ScaleXZ);
        m_WireframeShader->setUniform2f("uPos", m_Camera.getPos().x, m_Camera.getPos().z);
        m_WireframeShader->setUniformMatrix4fv("uMVP", mvp);
    }
    
    {
        ProfilerGPUEvent we("Terrain::Draw");

        auto scale_y = m_Map.getScaleSettings().ScaleY;
        m_Clipmap.BindAndDraw(m_Camera, scale_y);
    }
}

void TerrainRenderer::RenderShaded()
{
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        ProfilerGPUEvent we("Terrain::PrepareShaded");

        const float scale_xz = m_Map.getScaleSettings().ScaleXZ;

        const glm::mat4 mvp = m_Camera.getViewProjMatrix();

        m_ShadedShader->Bind();
        m_ShadedShader->setUniform1f("uL", scale_xz);
        m_ShadedShader->setUniform3f("uLightDir", m_Sky.getSunDir());
        m_ShadedShader->setUniform3f("uPos", m_Camera.getPos());
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

        m_ShadedShader->setUniform1f("uAerialDist", m_Sky.getAerialDistScale());

        m_Map.BindNormalmap(0);
        m_ShadedShader->setUniform1i("normalmap", 0);
        m_Map.BindShadowmap(1);
        m_ShadedShader->setUniform1i("shadowmap", 1);
        m_Map.BindMaterialmap(2);
        m_ShadedShader->setUniform1i("materialmap", 2);

        m_Material.BindAlbedo(3);
        m_ShadedShader->setUniform1i("albedo", 3);
        m_Material.BindNormal(4);
        m_ShadedShader->setUniform1i("normal", 4);

        m_Sky.BindIrradiance(5);
        m_ShadedShader->setUniform1i("irradiance", 5);
        m_Sky.BindPrefiltered(6);
        m_ShadedShader->setUniform1i("prefiltered", 6);
        m_Sky.BindAerial(7);
        m_ShadedShader->setUniform1i("aerial", 7);
    }
    
    {
        ProfilerGPUEvent we("Terrain::Draw");

        auto scale_y = m_Map.getScaleSettings().ScaleY;
        m_Clipmap.BindAndDraw(m_Camera, scale_y);
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