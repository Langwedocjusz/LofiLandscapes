#include "MaterialMapGenerator.h"

#include "Profiler.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

MaterialMapGenerator::MaterialMapGenerator(
        ResourceManager& manager, const MapGenerator& map)
    : m_ResourceManager(manager)
    , m_MaterialEditor(m_ResourceManager, "Material")
    , m_Map(map)
{
    m_Materialmap = m_ResourceManager.RequestTexture2D("Materialmap");
}

void MaterialMapGenerator::Init(int res, int wrap_type)
{
    m_Materialmap->Initialize(Texture2DSpec{
        res, res, GL_RGBA8, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        wrap_type,
        {1.0f, 0.0f, 0.0f, 0.0f}
    });

    m_Materialmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);


    //-----Setup material editor:
    const int max_layer_id = 4;

    m_MaterialEditor.RegisterShader("One material", "res/shaders/material_map/one_material.glsl");
    m_MaterialEditor.Attach<SliderIntTask>("One material", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select height", "res/shaders/material_map/select_height.glsl");
    m_MaterialEditor.Attach<SliderFloatTask>("Select height", "uHeightUpper", "Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select height", "uHeightLower", "Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select height", "uBlend", "Blending", 0.0f, 0.1f, 0.01f);
    m_MaterialEditor.Attach<SliderIntTask>("Select height", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select slope", "res/shaders/material_map/select_slope.glsl");
    m_MaterialEditor.Attach<SliderFloatTask>("Select slope", "uSlopeUpper", "Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select slope", "uSlopeLower", "Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select slope", "uBlend", "Blending", 0.0f, 0.1f, 0.01f);
    m_MaterialEditor.Attach<SliderIntTask>("Select slope", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select curvature", "res/shaders/material_map/select_curvature.glsl");
    m_MaterialEditor.Attach<SliderFloatTask>("Select curvature", "uCurvatureUpper", "Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select curvature", "uCurvatureLower", "Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select curvature", "uBlend", "Blending", 0.0f, 0.1f, 0.01f);
    m_MaterialEditor.Attach<SliderIntTask>("Select curvature", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select all", "res/shaders/material_map/select_all.glsl");
    m_MaterialEditor.Attach<SliderFloatTask>("Select all", "uHeightUpper", "Height Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select all", "uHeightLower", "Height Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select all", "uSlopeUpper", "Slope Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select all", "uSlopeLower", "Slope Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select all", "uCurvatureLower", "Curvature Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select all", "uCurvatureUpper", "Curvature Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select all", "uBlend", "Blending", 0.0f, 0.1f, 0.01f);
    m_MaterialEditor.Attach<SliderIntTask>("Select all", "uID", "Material id", 0, max_layer_id, 0);

    //Initial procedures:
    m_MaterialEditor.AddProcedureInstance("One material");
}

void MaterialMapGenerator::OnUpdate()
{
    if (!m_UpdateQueued) return;

    ProfilerGPUEvent we("Map::UpdateMaterial");

    const int res = m_Materialmap->getResolutionX();

    m_Map.BindHeightmap();

    m_Materialmap->BindImage(0, 0);
    m_MaterialEditor.OnDispatch(res);

    m_Materialmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    m_ResourceManager.RequestPreviewUpdate(m_Materialmap);

    m_UpdateQueued = false;
}

void MaterialMapGenerator::BindMaterialmap(int id) const
{
    m_Materialmap->Bind(id);
}

void MaterialMapGenerator::OnImGui(bool& open)
{
    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOFI_ICONS_MATERIALMAP "Material Map", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    ImGuiUtils::BeginGroupPanel("Material map procedures");
    m_UpdateQueued = m_MaterialEditor.OnImGui();
    ImGuiUtils::EndGroupPanel();

    ImGui::End();
}

void MaterialMapGenerator::OnSerialize(nlohmann::ordered_json& output)
{
    m_MaterialEditor.OnSerialize(output);
}

void MaterialMapGenerator::OnDeserialize(nlohmann::ordered_json& input)
{
    m_MaterialEditor.OnDeserialize(input[m_MaterialEditor.getName()]);

    m_UpdateQueued = true;
}
