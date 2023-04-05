#include "MapGenerator.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>

MapGenerator::MapGenerator()
    : m_NormalmapShader("res/shaders/terrain/normal.glsl")
    , m_ShadowmapShader("res/shaders/terrain/shadow.glsl")
    , m_MipShader("res/shaders/terrain/maximal_mip.glsl")
    , m_HeightEditor("Height")
    , m_MaterialEditor("Material")
{}

MapGenerator::~MapGenerator() {}

void MapGenerator::Init(int height_res, int shadow_res, int wrap_type) {
    //-----Initialize Textures
    //-----Heightmap

    TextureSpec heightmap_spec = TextureSpec{
        height_res, height_res, GL_R32F, GL_RGBA, 
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        wrap_type,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_Heightmap.Initialize(heightmap_spec);

    //Generate mips for heightmap:
    m_Heightmap.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    //-----Normal map: 
    TextureSpec normal_spec = TextureSpec{
        height_res, height_res, GL_RGBA8, GL_RGBA, 
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        wrap_type,
        //Pointing up (0,1,0), after compression -> (0.5, 1.0, 0.5):
        {0.5f, 1.0f, 0.5f, 1.0f}
    };

    m_Normalmap.Initialize(normal_spec);

    //-----Shadow map
    TextureSpec shadow_spec = TextureSpec{
        shadow_res, shadow_res, GL_R8, GL_RED, 
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        wrap_type,
        {1.0f, 1.0f, 1.0f, 1.0f}
    };

    m_Shadowmap.Initialize(shadow_spec);

    //--Material map
    const int material_res = height_res;

    TextureSpec material_spec = TextureSpec{
        material_res, material_res, GL_RGBA8, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        wrap_type,
        {0.0f, 0.0f, 0.0f, 1.0f}
    };

    m_Materialmap.Initialize(material_spec);

    //-----Setup heightmap editor:
    std::vector<std::string> labels{ "Average", "Add", "Subtract" };

    m_HeightEditor.RegisterShader("Const Value", "res/shaders/terrain/const_val.glsl");
    m_HeightEditor.AttachSliderFloat("Const Value", "uValue", "Value", 0.0, 1.0, 0.0);

    m_HeightEditor.RegisterShader("FBM", "res/shaders/terrain/fbm.glsl");
    m_HeightEditor.AttachConstInt("FBM", "uResolution", height_res);
    m_HeightEditor.AttachSliderInt("FBM", "uOctaves", "Octaves", 1, 16, 8);
    m_HeightEditor.AttachSliderFloat("FBM", "uScale", "Scale", 1.0, 64.0, 32.0);
    m_HeightEditor.AttachSliderFloat("FBM", "uRoughness", "Roughness", 0.0, 1.0, 0.5);
    m_HeightEditor.AttachGLEnum("FBM", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.AttachSliderFloat("FBM", "uWeight", "Weight", 0.0, 1.0, 1.0);

    m_HeightEditor.RegisterShader("Voronoi", "res/shaders/terrain/voronoi.glsl");
    m_HeightEditor.AttachConstInt("Voronoi", "uResolution", height_res);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uScale", "Scale", 1.0, 64.0, 8.0);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uRandomness", "Randomness", 0.0, 1.0, 1.0);

    std::vector<std::string> voro_types{ "F1", "F2", "F2_F1" };
    m_HeightEditor.AttachGLEnum("Voronoi", "uVoronoiType", "Type", voro_types);

    m_HeightEditor.AttachGLEnum("Voronoi", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uWeight", "Weight", 0.0, 1.0, 1.0);

    m_HeightEditor.RegisterShader("Curves", "res/shaders/terrain/curves.glsl");
    m_HeightEditor.AttachSliderFloat("Curves", "uExponent", "Exponent", 0.1, 4.0, 1.0);

    m_HeightEditor.RegisterShader("Radial cutoff", "res/shaders/terrain/radial_cutoff.glsl");
    m_HeightEditor.AttachConstInt("Radial cutoff", "uResolution", height_res);
    m_HeightEditor.AttachSliderFloat("Radial cutoff", "uBias", "Bias", 0.0, 1.0, 0.5);
    m_HeightEditor.AttachSliderFloat("Radial cutoff", "uSlope", "Slope", 0.0, 10.0, 4.0);

    //Initial procedures:
    m_HeightEditor.AddProcedureInstance("Const Value");
    m_HeightEditor.AddProcedureInstance("FBM");
    m_HeightEditor.AddProcedureInstance("Radial cutoff");

    //-----Setup material editor:
    const int max_layer_id = 4;

    m_MaterialEditor.RegisterShader("One material", "res/shaders/terrain/one_material.glsl");
    m_MaterialEditor.AttachSliderInt("One material", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select height", "res/shaders/terrain/select_height.glsl");
    m_MaterialEditor.AttachConstInt("Select height", "uResolution", material_res);
    m_MaterialEditor.AttachSliderFloat("Select height", "uHeightUpper", "Upper", 0.0, 1.0, 1.0);
    m_MaterialEditor.AttachSliderFloat("Select height", "uHeightLower", "Lower", 0.0, 1.0, 0.0);
    m_MaterialEditor.AttachSliderFloat("Select height", "uBlend", "Blending", 0.0, 0.1, 0.01);
    m_MaterialEditor.AttachSliderInt("Select height", "uID", "Material id", 0, max_layer_id, 0);

    m_MaterialEditor.RegisterShader("Select slope", "res/shaders/terrain/select_slope.glsl");
    m_MaterialEditor.AttachConstInt("Select slope", "uResolution", material_res);
    m_MaterialEditor.AttachSliderFloat("Select slope", "uSlopeUpper", "Upper", 0.0, 1.0, 1.0);
    m_MaterialEditor.AttachSliderFloat("Select slope", "uSlopeLower", "Lower", 0.0, 1.0, 0.0);
    m_MaterialEditor.AttachSliderFloat("Select slope", "uBlend", "Blending", 0.0, 0.1, 0.01);
    m_MaterialEditor.AttachSliderInt("Select slope", "uID", "Material id", 0, max_layer_id, 0);

    //Initial procedures:
    m_MaterialEditor.AddProcedureInstance("One material");

    //-----Set update flags
    m_UpdateFlags = Height | Normal | Shadow | Material;

    //-----Mipmap related things
    //Check if heightmap resolution is a power of 2
    int res = m_Heightmap.getSpec().ResolutionX;

    if ((res & (res - 1)) != 0) {
        std::cerr << "Heightmap res is not a power of 2!" << '\n';
        return;
    }

    //Do the same for shadowmap
    int res_s = m_Shadowmap.getSpec().ResolutionX;

    if ((res_s & (res_s - 1)) != 0) {
        std::cerr << "Shadowmap res is not a power of 2!" << '\n';
        return;
    }

    auto log2 = [](int value) {
        int copy = value, result = 0;
        while (copy >>= 1) ++result;
        return result;
    };

    m_MipLevels = log2(res);

    m_ShadowSettings.MipOffset = log2(res / res_s);
}

void MapGenerator::UpdateHeight() { 
    const int res = m_Heightmap.getSpec().ResolutionX;

    m_Heightmap.BindImage(0, 0);
    m_HeightEditor.OnDispatch(res);

    m_Heightmap.Bind();

    GenMaxMips();
}

void MapGenerator::UpdateNormal() {
    const int res = m_Normalmap.getSpec().ResolutionX;

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

void MapGenerator::UpdateShadow(const glm::vec3& sun_dir) {
    const int res = m_Shadowmap.getSpec().ResolutionX;

    m_Heightmap.Bind();
    m_Shadowmap.BindImage(0, 0);
 
    m_ShadowmapShader.Bind();
    m_ShadowmapShader.setUniform1i("uResolution", res);
    m_ShadowmapShader.setUniform3f("uSunDir", sun_dir);
    m_ShadowmapShader.setUniform1f("uScaleXZ", m_ScaleSettings.ScaleXZ);
    m_ShadowmapShader.setUniform1f("uScaleY", m_ScaleSettings.ScaleY);
    
    m_ShadowmapShader.setUniform1i("uMips", m_MipLevels);
    m_ShadowmapShader.setUniform1i("uMipOffset", m_ShadowSettings.MipOffset);
    m_ShadowmapShader.setUniform1i("uMinLvl", m_ShadowSettings.MinLevel);
    m_ShadowmapShader.setUniform1i("uStartCell", m_ShadowSettings.StartCell);
    m_ShadowmapShader.setUniform1f("uNudgeFactor", m_ShadowSettings.NudgeFac);
    m_ShadowmapShader.setUniform1i("uSoftShadows", m_ShadowSettings.Soft);
    m_ShadowmapShader.setUniform1f("uSharpness", m_ShadowSettings.Sharpness);

    glDispatchCompute(res/32, res/32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void MapGenerator::UpdateMaterial() {
    const int res = m_Materialmap.getSpec().ResolutionX;

    m_Heightmap.Bind();

    m_Materialmap.BindImage(0, 0);
    m_MaterialEditor.OnDispatch(res);
}

void MapGenerator::GenMaxMips() {
    if (m_MipLevels == 0) return;

    const int res = m_Heightmap.getSpec().ResolutionX;

    for (int i = 0; i < m_MipLevels; i++) {
        m_MipShader.Bind();

        //Higher res - read from this
        m_Heightmap.BindImage(1, i);
        //Lower res - modify this
        m_Heightmap.BindImage(0, i+1);

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
    m_Heightmap.Bind(id);
}

void MapGenerator::BindNormalmap(int id) const {
    m_Normalmap.Bind(id);
}

void MapGenerator::BindShadowmap(int id) const {
    m_Shadowmap.Bind(id);
}

void MapGenerator::BindMaterialmap(int id) const {
    m_Materialmap.Bind(id);
}

void MapGenerator::ImGuiTerrain(bool &open, bool update_shadows) {
    ScaleSettings temp_s = m_ScaleSettings;

    ImGui::Begin("Terrain editor", &open);

    ImGui::Text("Scale:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::SliderFloat("Scale xz", &(temp_s.ScaleXZ), 0.0f, 400.0f);
    ImGuiUtils::SliderFloat("Scale y" , &(temp_s.ScaleY ), 0.0f, 100.0f);
    ImGui::Columns(1, "###col");
    ImGuiUtils::Separator();

    bool scale_changed = (temp_s != m_ScaleSettings);

    ImGui::Text("Heightmap procedures:");

    bool height_changed = m_HeightEditor.OnImGui();

    ImGui::End();

    if (height_changed) {
        m_UpdateFlags = m_UpdateFlags | Height | Normal | Material;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }

    else if (scale_changed) {
        m_ScaleSettings = temp_s;
        m_UpdateFlags = m_UpdateFlags | Normal;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }

}

void MapGenerator::ImGuiShadowmap(bool &open, bool update_shadows) {
    ShadowmapSettings temp = m_ShadowSettings;
    AOSettings temp2 = m_AOSettings;

    ImGui::Begin("Shadow/AO settings", &open);

    ImGui::Text("Shadowmap Settings:");
    ImGui::Spacing();

    ImGui::Columns(2, "###col");
    ImGuiUtils::SliderInt("Min level", &temp.MinLevel, 0, 12);
    ImGuiUtils::SliderFloat("Nudge fac", &temp.NudgeFac, 1.005, 1.1);
    ImGuiUtils::Checkbox("Soft Shadows", &temp.Soft);
    ImGuiUtils::SliderFloat("Sharpness", &temp.Sharpness, 0.1, 3.0);
    ImGui::Columns(1, "###col");

    ImGuiUtils::Separator();
    ImGui::Text("AO Settings:");
    ImGui::Spacing();

    ImGui::Columns(2, "###col");
    ImGuiUtils::SliderInt("AO Samples", &temp2.Samples, 1, 64);
    ImGuiUtils::SliderFloat("AO Radius", &temp2.R, 0.0, 0.1);
    ImGui::Columns(1, "###col");

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
    ImGui::Begin("Material Map", &open);

    bool material_changed = m_MaterialEditor.OnImGui();

    ImGui::End();

    if (material_changed)
        m_UpdateFlags = m_UpdateFlags | Material;
}

void MapGenerator::RequestShadowUpdate() {
    m_UpdateFlags = m_UpdateFlags | Shadow;
}

bool MapGenerator::GeometryShouldUpdate() {
    //If height changed, then normal must also change, but it is possible
    //to change normals without height by changing the scale
    return (m_UpdateFlags & Normal) != None;
}

//Settings structs operator overloads:

bool operator==(const ScaleSettings& lhs, const ScaleSettings& rhs) {
    return (lhs.ScaleXZ == rhs.ScaleXZ) && (lhs.ScaleY == rhs.ScaleY);
}

bool operator!=(const ScaleSettings& lhs, const ScaleSettings& rhs) {
    return !(lhs==rhs);
}

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