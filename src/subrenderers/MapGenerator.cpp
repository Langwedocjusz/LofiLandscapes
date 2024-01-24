#include "MapGenerator.h"

#include "Profiler.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include <iostream>

MapGenerator::MapGenerator(ResourceManager& manager)
    : m_ResourceManager(manager)
    , m_HeightEditor(manager, "Height")
    , m_MaterialEditor(manager, "Material")
{
    m_NormalmapShader = m_ResourceManager.RequestComputeShader("res/shaders/terrain/normal.glsl");
    m_ShadowmapShader = m_ResourceManager.RequestComputeShader("res/shaders/terrain/shadow.glsl");
    m_MipShader       = m_ResourceManager.RequestComputeShader("res/shaders/terrain/maximal_mip.glsl");

    m_Heightmap   = m_ResourceManager.RequestTexture2D("Heightmap");
    m_Normalmap   = m_ResourceManager.RequestTexture2D("Normalmap");
    m_Shadowmap   = m_ResourceManager.RequestTexture2D("Shadowmap");
    m_Materialmap = m_ResourceManager.RequestTexture2D("Materialmap");
}

void MapGenerator::Init(int height_res, int shadow_res, int wrap_type) {
    //-----Initialize Textures
    //-----Heightmap

    m_Heightmap->Initialize(Texture2DSpec{
        height_res, height_res, GL_R32F, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        wrap_type,
        {0.0f, 0.0f, 0.0f, 0.0f}
    });

    m_Heightmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    //-----Normal map: 
    m_Normalmap->Initialize(Texture2DSpec{
        height_res, height_res, GL_RGBA8, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        wrap_type,
        //Pointing up (0,1,0), after compression -> (0.5, 1.0, 0.5):
        {0.5f, 1.0f, 0.5f, 1.0f}
    });

    m_Normalmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    //-----Shadow map
    m_Shadowmap->Initialize(Texture2DSpec{
        shadow_res, shadow_res, GL_R8, GL_RED,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        wrap_type,
        {1.0f, 1.0f, 1.0f, 1.0f}
    });

    m_Shadowmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    //--Material map
    m_Materialmap->Initialize(Texture2DSpec{
        height_res, height_res, GL_RGBA8, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        wrap_type,
        {1.0f, 0.0f, 0.0f, 0.0f}
    });

    m_Materialmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    //-----Setup heightmap editor:
    std::vector<std::string> labels{ "Average", "Add", "Subtract" };

    m_HeightEditor.RegisterShader("Const Value", "res/shaders/terrain/const_val.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Const Value", "uValue", "Value", 0.0f, 1.0f, 0.0f);

    m_HeightEditor.RegisterShader("FBM", "res/shaders/terrain/fbm.glsl");
    m_HeightEditor.Attach<SliderIntTask>("FBM", "uOctaves", "Octaves", 1, 16, 8);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uScale", "Scale", 1.0f, 64.0f, 32.0f);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uRoughness", "Roughness", 0.0f, 1.0f, 0.5f);
    m_HeightEditor.Attach<GLEnumTask>("FBM", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uWeight", "Weight", 0.0f, 1.0f, 1.0f);

    m_HeightEditor.RegisterShader("Voronoi", "res/shaders/terrain/voronoi.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uScale", "Scale", 1.0f, 64.0f, 8.0f);
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uRandomness", "Randomness", 0.0f, 1.0f, 1.0f);

    std::vector<std::string> voro_types{ "F1", "F2", "F2_F1" };
    m_HeightEditor.Attach<GLEnumTask>("Voronoi", "uVoronoiType", "Type", voro_types);

    m_HeightEditor.Attach<GLEnumTask>("Voronoi", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uWeight", "Weight", 0.0f, 1.0f, 1.0f);

    m_HeightEditor.RegisterShader("Curves", "res/shaders/terrain/curves.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Curves", "uExponent", "Exponent", 0.1f, 4.0f, 1.0f);

    m_HeightEditor.RegisterShader("Radial cutoff", "res/shaders/terrain/radial_cutoff.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Radial cutoff", "uBias", "Bias", 0.0f, 1.0f, 0.5f);
    m_HeightEditor.Attach<SliderFloatTask>("Radial cutoff", "uSlope", "Slope", 0.0f, 10.0f, 4.0f);

    //Initial procedures:
    m_HeightEditor.AddProcedureInstance("Const Value");
    m_HeightEditor.AddProcedureInstance("FBM");
    m_HeightEditor.AddProcedureInstance("Radial cutoff");

    //-----Setup material editor:
    const int max_layer_id = 4;

    m_MaterialEditor.RegisterShader("One material", "res/shaders/terrain/one_material.glsl");
    m_MaterialEditor.Attach<SliderIntTask>("One material", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select height", "res/shaders/terrain/select_height.glsl");
    m_MaterialEditor.Attach<SliderFloatTask>("Select height", "uHeightUpper", "Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select height", "uHeightLower", "Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select height", "uBlend", "Blending", 0.0f, 0.1f, 0.01f);
    m_MaterialEditor.Attach<SliderIntTask>("Select height", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select slope", "res/shaders/terrain/select_slope.glsl");
    m_MaterialEditor.Attach<SliderFloatTask>("Select slope", "uSlopeUpper", "Upper", 0.0f, 1.0f, 1.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select slope", "uSlopeLower", "Lower", 0.0f, 1.0f, 0.0f);
    m_MaterialEditor.Attach<SliderFloatTask>("Select slope", "uBlend", "Blending", 0.0f, 0.1f, 0.01f);
    m_MaterialEditor.Attach<SliderIntTask>("Select slope", "uID", "Material id", 0, max_layer_id, 0);

    //Initial procedures:
    m_MaterialEditor.AddProcedureInstance("One material");

    //-----Set update flags
    m_UpdateFlags = Height | Normal | Shadow | Material;

    //-----Mipmap related things
    
    //Nice utilities:
    //http://www.graphics.stanford.edu/~seander/bithacks.html

    auto isPowerOf2 = [](int value) {
        return (value & (value - 1)) == 0;
    };

    auto log2 = [](int value) {
        int copy = value, result = 0;
        while (copy >>= 1) ++result;
        return result;
    };
    

    if (!isPowerOf2(height_res))
        std::cerr << "Heightmap res is not a power of 2!" << '\n';

    if (!isPowerOf2(shadow_res))
        std::cerr << "Shadowmap res is not a power of 2!" << '\n';


    m_MipLevels = log2(height_res);
    m_ShadowSettings.MipOffset = log2(height_res / shadow_res);
}

void MapGenerator::UpdateHeight() {
    ProfilerGPUEvent we("Map::UpdateHeight");

    const int res = m_Heightmap->getResolutionX();

    m_Heightmap->BindImage(0, 0);
    m_HeightEditor.OnDispatch(res);

    m_Heightmap->Bind();

    GenMaxMips();

    m_ResourceManager.RequestPreviewUpdate(m_Heightmap);
}

void MapGenerator::UpdateNormal() {
    ProfilerGPUEvent we("Map::UpdateNormal");

    const int res = m_Normalmap->getResolutionX();

    m_Heightmap->Bind();
    m_Normalmap->BindImage(0, 0);
 
    m_NormalmapShader->Bind();
    m_NormalmapShader->setUniform1f("uScaleXZ", m_ScaleXZ);
    m_NormalmapShader->setUniform1f("uScaleY" , m_ScaleY );

    m_NormalmapShader->setUniform1i("uAOSamples", m_AOSettings.Samples);
    m_NormalmapShader->setUniform1f("uAOR", m_AOSettings.R);

    m_NormalmapShader->Dispatch(res, res, 1);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_Normalmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    m_ResourceManager.RequestPreviewUpdate(m_Normalmap);
}

void MapGenerator::UpdateShadow(const glm::vec3& sun_dir) {
    ProfilerGPUEvent we("Map::UpdateShadow");

    const int res = m_Shadowmap->getResolutionX();

    m_Heightmap->Bind();
    m_Shadowmap->BindImage(0, 0);
 
    m_ShadowmapShader->Bind();
    m_ShadowmapShader->setUniform1i("uResolution", res);
    m_ShadowmapShader->setUniform3f("uSunDir", sun_dir);
    m_ShadowmapShader->setUniform1f("uScaleXZ", m_ScaleXZ);
    m_ShadowmapShader->setUniform1f("uScaleY", m_ScaleY);
    
    m_ShadowmapShader->setUniform1i("uMips", m_MipLevels);
    m_ShadowmapShader->setUniform1i("uMipOffset", m_ShadowSettings.MipOffset);
    m_ShadowmapShader->setUniform1i("uMinLvl", m_ShadowSettings.MinLevel);
    m_ShadowmapShader->setUniform1i("uStartCell", m_ShadowSettings.StartCell);
    m_ShadowmapShader->setUniform1f("uNudgeFactor", m_ShadowSettings.NudgeFac);
    m_ShadowmapShader->setUniform1i("uSoftShadows", m_ShadowSettings.Soft);
    m_ShadowmapShader->setUniform1f("uSharpness", m_ShadowSettings.Sharpness);

    m_ShadowmapShader->Dispatch(res, res, 1);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_Shadowmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    m_ResourceManager.RequestPreviewUpdate(m_Shadowmap);
}

void MapGenerator::UpdateMaterial() {
    ProfilerGPUEvent we("Map::UpdateMaterial");

    const int res = m_Materialmap->getResolutionX();

    m_Heightmap->Bind();

    m_Materialmap->BindImage(0, 0);
    m_MaterialEditor.OnDispatch(res);

    m_Materialmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    m_ResourceManager.RequestPreviewUpdate(m_Materialmap);
}

void MapGenerator::GenMaxMips() {
    if (m_MipLevels == 0) return;

    const int res = m_Heightmap->getResolutionX();

    for (int i = 0; i < m_MipLevels; i++) {
        m_MipShader->Bind();

        //Higher res - read from this
        m_Heightmap->BindImage(1, i);
        //Lower res - modify this
        m_Heightmap->BindImage(0, i+1);

        //We don't set uniforms for binding ids since they are set in shader code

        //Number of work groups for compute shader
        const int size = std::max((res / int(pow(2, i + 1))) / 32, 1);

        glDispatchCompute(size, size, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }
}

void MapGenerator::Update(const glm::vec3& sun_dir) {
    if ((m_UpdateFlags & Height) != None)
        UpdateHeight();

    if ((m_UpdateFlags & Normal) != None)
        UpdateNormal();

    if ((m_UpdateFlags & Shadow) != None)
        UpdateShadow(sun_dir);

    if ((m_UpdateFlags & Material) != None)
        UpdateMaterial();

    m_UpdateFlags = None;
}

void MapGenerator::BindHeightmap(int id) const {
    m_Heightmap->Bind(id);
}

void MapGenerator::BindNormalmap(int id) const {
    m_Normalmap->Bind(id);
}

void MapGenerator::BindShadowmap(int id) const {
    m_Shadowmap->Bind(id);
}

void MapGenerator::BindMaterialmap(int id) const {
    m_Materialmap->Bind(id);
}

void MapGenerator::ImGuiTerrain(bool &open, bool update_shadows) {
    float scale_xz = m_ScaleXZ;
    float scale_y = m_ScaleY;

    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOFI_ICONS_TERRAIN "Terrain editor", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    ImGuiUtils::BeginGroupPanel("Scale");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderFloat("Scale xz", &(scale_xz), 0.0f, 400.0f);
    ImGuiUtils::ColSliderFloat("Scale y" , &(scale_y), 0.0f, 100.0f);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    bool scale_changed = (scale_xz != m_ScaleXZ) || (scale_y != m_ScaleY);

    ImGuiUtils::BeginGroupPanel("Heightmap procedures");

    bool height_changed = m_HeightEditor.OnImGui();

    ImGuiUtils::EndGroupPanel();

    ImGui::End();

    if (height_changed) {
        m_UpdateFlags = m_UpdateFlags | Height | Normal | Material;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }

    else if (scale_changed) {
        m_ScaleXZ = scale_xz;
        m_ScaleY = scale_y;

        m_UpdateFlags = m_UpdateFlags | Normal;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }

}

void MapGenerator::ImGuiShadowmap(bool &open, bool update_shadows) {
    ShadowmapSettings temp = m_ShadowSettings;
    AOSettings temp2 = m_AOSettings;

    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOFI_ICONS_SHADOW "Shadow/AO settings", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    ImGuiUtils::BeginGroupPanel("Shadowmap settings:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderInt("Min level", &temp.MinLevel, 0, 12);
    ImGuiUtils::ColSliderFloat("Nudge fac", &temp.NudgeFac, 1.005, 1.1);
    ImGuiUtils::ColCheckbox("Soft Shadows", &temp.Soft);
    ImGuiUtils::ColSliderFloat("Sharpness", &temp.Sharpness, 0.1, 3.0);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGuiUtils::BeginGroupPanel("AO settings:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderInt("AO Samples", &temp2.Samples, 1, 64);
    ImGuiUtils::ColSliderFloat("AO Radius", &temp2.R, 0.0, 0.1);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGui::End();

    if (temp2 != m_AOSettings) {
        m_AOSettings = temp2;
        m_UpdateFlags = m_UpdateFlags | Normal;
    }

    if (temp != m_ShadowSettings) {
        temp.StartCell = pow(2, temp.MinLevel);

        m_ShadowSettings = temp;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }
}

void MapGenerator::ImGuiMaterials(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOFI_ICONS_MATERIALMAP "Material Map", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    bool material_changed = m_MaterialEditor.OnImGui();

    ImGui::End();

    if (material_changed)
        m_UpdateFlags = m_UpdateFlags | Material;
}

void MapGenerator::RequestShadowUpdate() const {
    m_UpdateFlags = m_UpdateFlags | Shadow;
}

bool MapGenerator::GeometryShouldUpdate() {
    //If height changed, then normal must also change, but it is possible
    //to change normals without height by changing the scale
    return (m_UpdateFlags & Normal) != None;
}

void MapGenerator::OnSerialize(nlohmann::ordered_json& output)
{
    output["Scale XZ"] = m_ScaleXZ;
    output["Scale Y"] = m_ScaleY;

    m_HeightEditor.OnSerialize(output);
    m_MaterialEditor.OnSerialize(output);
}

void MapGenerator::OnDeserialize(nlohmann::ordered_json& input)
{
    m_ScaleXZ = input["Scale XZ"];
    m_ScaleY = input["Scale Y"];

    m_HeightEditor.OnDeserialize(input[m_HeightEditor.getName()]);
    m_MaterialEditor.OnDeserialize(input[m_MaterialEditor.getName()]);

    m_UpdateFlags = Height | Normal | Shadow | Material;
}

//Settings structs operator overloads:

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs) {
    return (lhs.MinLevel == rhs.MinLevel) && (lhs.StartCell == rhs.StartCell)
        && (lhs.NudgeFac == rhs.NudgeFac) && (lhs.Soft == rhs.Soft)
        && (lhs.Sharpness == rhs.Sharpness);
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