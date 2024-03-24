#include "TerrainRenderer.h"

#include "imgui.h"
#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include "Profiler.h"

#include "glad/glad.h"

TerrainRenderer::TerrainRenderer(ResourceManager& manager, const PerspectiveCamera& cam,
                                 const MapGenerator& map, const MaterialGenerator& material,
                                 const MaterialMapGenerator& material_map, const SkyRenderer& sky)
    : m_ResourceManager(manager)
    , m_Camera(cam)
    , m_Map(map)
    , m_Material(material)
    , m_MaterialMap(material_map)
    , m_Sky(sky)
{
    m_ShadedShader    = m_ResourceManager.RequestVertFragShader("res/shaders/shaded.vert", "res/shaders/shaded.frag");
    m_WireframeShader = m_ResourceManager.RequestVertFragShader("res/shaders/wireframe.vert", "res/shaders/wireframe.frag");

    m_DisplaceShader = m_ResourceManager.RequestComputeShader("res/shaders/displace.glsl");
}

TerrainRenderer::~TerrainRenderer() {}

void TerrainRenderer::Init(uint32_t subdivisions, uint32_t levels)
{
    m_Clipmap.Init(subdivisions, levels);
}

void TerrainRenderer::Update()
{
    ProfilerGPUEvent we("Terrain::Update (Displace)");

    m_Map.BindHeightmap();

    const glm::vec2 curr{ m_Camera.getPos().x, m_Camera.getPos().z };

    m_DisplaceShader->Bind();
    m_DisplaceShader->setUniform2f("uPos", curr);
    m_DisplaceShader->setUniform1f("uScaleXZ", m_Map.getScaleXZ());
    m_DisplaceShader->setUniform1f("uScaleY", m_Map.getScaleY());

    const uint32_t binding_id = 1;

    if (m_UpdateAll)
    {
        m_Clipmap.RunCompute(m_DisplaceShader, binding_id);
    }

    else
    {
        const glm::vec2 prev{ m_Camera.getPrevPos().x, m_Camera.getPrevPos().z };

        m_Clipmap.RunCompute(m_DisplaceShader, binding_id, curr, prev);
    }

    m_UpdateAll = false;
}

void TerrainRenderer::RequestFullUpdate()
{
    m_UpdateAll = true;
}

void TerrainRenderer::RenderWireframe() {
    ProfilerGPUEvent we("Terrain::Draw");

    const glm::mat4 mvp = m_Camera.getViewProjMatrix();

    m_WireframeShader->Bind();
    m_WireframeShader->setUniform2f("uPos", m_Camera.getPos().x, m_Camera.getPos().z);
    m_WireframeShader->setUniformMatrix4fv("uMVP", mvp);
    
    auto scale_y = m_Map.getScaleY();

    const glm::vec3 grid_color{ 1.0f, 1.0f, 1.0f };
    const glm::vec3 trim_color{ 1.0f, 0.6f, 0.0f };
    const glm::vec3 fill_color{ 1.0f, 0.0f, 0.6f };

    m_WireframeShader->setUniform3f("uCol", grid_color);

    for (const auto& grid : m_Clipmap.getGrids())
    {
        if (m_Camera.IsInFrustum(grid.BoundingBox, scale_y))
        {
            grid.Draw();
        }
    }

    m_WireframeShader->setUniform3f("uCol", fill_color);

    for (const auto& fill : m_Clipmap.getFills())
    {
        fill.Draw();
    }

    m_WireframeShader->setUniform3f("uCol", trim_color);

    for (const auto& trim : m_Clipmap.getTrims())
    {
        trim.Draw();
    }
}

void TerrainRenderer::RenderShaded()
{
    ProfilerGPUEvent we("Terrain::Draw");

    const glm::mat4 mvp = m_Camera.getViewProjMatrix();

    m_ShadedShader->Bind();
    m_ShadedShader->setUniform1f("uScaleXZ", m_Map.getScaleXZ());
    m_ShadedShader->setUniform3f("uLightDir", m_Sky.getSunDir());
    m_ShadedShader->setUniform3f("uPos", m_Camera.getPos());
    m_ShadedShader->setUniformMatrix4fv("uMVP", mvp);
    m_ShadedShader->setUniform1i("uShadow", int(m_Shadows));
    m_ShadedShader->setUniform1i("uMaterial", int(m_Materials));
    m_ShadedShader->setUniform1i("uFixTiling", int(m_FixTiling));
    m_ShadedShader->setUniform1i("uFog", int(m_Fog));
    m_ShadedShader->setUniform3f("uSunCol", m_Sky.getSunCol());
    //m_ShadedShader->setUniform1f("uSunCol", 1.0f);
    m_ShadedShader->setUniform1f("uSunStr", m_SunStr);
    m_ShadedShader->setUniform1f("uSkyDiff", m_SkyDiff);
    m_ShadedShader->setUniform1f("uSkySpec", m_SkySpec);
    m_ShadedShader->setUniform1f("uRefStr", m_RefStr);
    m_ShadedShader->setUniform1f("uTilingFactor", m_TilingFactor);
    m_ShadedShader->setUniform1f("uNormalStrength", m_NormalStrength);

    m_ShadedShader->setUniform1f("uAerialDist", m_Sky.getAerialDistScale());

    m_Map.BindNormalmap(0);
    m_ShadedShader->setUniformSampler2D("normalmap", 0);
    m_Map.BindShadowmap(1);
    m_ShadedShader->setUniformSampler2D("shadowmap", 1);
    m_MaterialMap.BindMaterialmap(2);
    m_ShadedShader->setUniformSampler2D("materialmap", 2);

    m_Material.BindAlbedo(3);
    m_ShadedShader->setUniformSampler2DArray("albedo", 3);
    m_Material.BindNormal(4);
    m_ShadedShader->setUniformSampler2DArray("normal", 4);

    m_Sky.BindIrradiance(5);
    m_ShadedShader->setUniformSamplerCube("irradiance", 5);
    m_Sky.BindPrefiltered(6);
    m_ShadedShader->setUniformSamplerCube("prefiltered", 6);
    m_Sky.BindAerial(7);
    m_ShadedShader->setUniformSampler3D("aerial", 7);

    auto scale_y = m_Map.getScaleY();
    
    m_Clipmap.Draw(m_Camera, scale_y);
}

void TerrainRenderer::OnImGui(bool& open) 
{
    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOFI_ICONS_LIGHTING "Lighting", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    const bool shadows = m_Shadows;

    ImGuiUtils::BeginGroupPanel("Lighting options:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderFloat("Sun", &m_SunStr, 0.0, 4.0);
    ImGuiUtils::ColSliderFloat("Sky Diffuse", &m_SkyDiff, 0.0, 1.0);
    ImGuiUtils::ColSliderFloat("Sky Specular", &m_SkySpec, 0.0, 1.0);
    ImGuiUtils::ColSliderFloat("Reflected", &m_RefStr, 0.0, 1.0);
    ImGuiUtils::ColCheckbox("Shadows", &m_Shadows);
    ImGuiUtils::ColCheckbox("Materials", &m_Materials);
    ImGuiUtils::ColCheckbox("Render fog", &m_Fog);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    if ((shadows != m_Shadows) && m_Shadows) 
        m_Map.RequestShadowUpdate();

    ImGuiUtils::BeginGroupPanel("Material options:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColCheckbox("Fix Tiling", &m_FixTiling);
    ImGuiUtils::ColSliderFloat("Tiling Factor", &m_TilingFactor, 0.0, 128.0);
    ImGuiUtils::ColSliderFloat("Normal Strength", &m_NormalStrength, 0.0, 1.0);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGuiUtils::BeginGroupPanel("Background:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColColorEdit3("ClearColor", &m_ClearColor);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGui::End();
}
