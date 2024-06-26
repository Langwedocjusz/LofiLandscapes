#include "MapGenerator.h"

#include "Profiler.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include <iostream>

MapGenerator::MapGenerator(ResourceManager& manager)
    : m_ResourceManager(manager)
    , m_HeightEditor(m_ResourceManager, "Height")
{
    m_NormalmapShader = m_ResourceManager.RequestComputeShader("res/shaders/map/normal.glsl");
    m_ShadowmapShader = m_ResourceManager.RequestComputeShader("res/shaders/map/shadow.glsl");
    m_MipShader       = m_ResourceManager.RequestComputeShader("res/shaders/map/maximal_mip.glsl");

    m_Heightmap   = m_ResourceManager.RequestTexture2D("Heightmap");
    m_Normalmap   = m_ResourceManager.RequestTexture2D("Normalmap");
    m_Shadowmap   = m_ResourceManager.RequestTexture2D("Shadowmap");
}

void MapGenerator::Init(int height_res, int shadow_res, int wrap_type)
{
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


    //-----Setup heightmap editor:
    std::vector<std::string> labels{ "Average", "Add", "Subtract" };

    m_HeightEditor.RegisterShader("Const Value", "res/shaders/map/const_val.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Const Value", "uValue", "Value", 0.0f, 1.0f, 0.0f);

    m_HeightEditor.RegisterShader("FBM", "res/shaders/map/fbm.glsl");
    m_HeightEditor.Attach<SliderIntTask>("FBM", "uOctaves", "Octaves", 1, 16, 8);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uScale", "Scale", 1.0f, 64.0f, 32.0f);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uRoughness", "Roughness", 0.0f, 1.0f, 0.5f);
    m_HeightEditor.Attach<GLEnumTask>("FBM", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.Attach<SliderFloatTask>("FBM", "uWeight", "Weight", 0.0f, 1.0f, 1.0f);

    m_HeightEditor.RegisterShader("Advanced FBM", "res/shaders/map/advanced_fbm.glsl");

    const std::vector<std::string> noise_types{ "Value", "Perlin" };
    m_HeightEditor.Attach<GLEnumTask>("Advanced FBM", "uNoiseType", "Noise Type", noise_types);

    m_HeightEditor.Attach<SliderIntTask>("Advanced FBM", "uOctaves", "Octaves", 1, 16, 8);
    m_HeightEditor.Attach<SliderFloatTask>("Advanced FBM", "uScale", "Scale", 1.0f, 64.0f, 32.0f);
    m_HeightEditor.Attach<SliderFloatTask>("Advanced FBM", "uRoughness", "Roughness", 0.0f, 1.0f, 0.5f);
    m_HeightEditor.Attach<SliderFloatTask>("Advanced FBM", "uLacunarity", "Lacunarity", 0.0f, 5.0f, 2.0f);
    m_HeightEditor.Attach<SliderFloatTask>("Advanced FBM", "uAltitudeErosion", "Altitude Erosion", 0.0f, 1.0f, 0.0f);
    m_HeightEditor.Attach<SliderFloatTask>("Advanced FBM", "uSlopeErosion", "Slope Erosion", 0.0f, 1.0f, 0.0f);
    m_HeightEditor.Attach<SliderFloatTask>("Advanced FBM", "uConcaveErosion", "Concave Erosion", 0.0f, 1.0f, 0.0f);
    m_HeightEditor.Attach<GLEnumTask>("Advanced FBM", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.Attach<SliderFloatTask>("Advanced FBM", "uWeight", "Weight", 0.0f, 1.0f, 1.0f);

    m_HeightEditor.RegisterShader("Voronoi", "res/shaders/map/voronoi.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uScale", "Scale", 1.0f, 64.0f, 8.0f);
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uRandomness", "Randomness", 0.0f, 1.0f, 1.0f);

    std::vector<std::string> voro_types{ "F1", "F2", "F2_F1" };
    m_HeightEditor.Attach<GLEnumTask>("Voronoi", "uVoronoiType", "Type", voro_types);

    m_HeightEditor.Attach<GLEnumTask>("Voronoi", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.Attach<SliderFloatTask>("Voronoi", "uWeight", "Weight", 0.0f, 1.0f, 1.0f);

    m_HeightEditor.RegisterShader("Curves", "res/shaders/map/curves.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Curves", "uExponent", "Exponent", 0.1f, 4.0f, 1.0f);

    m_HeightEditor.RegisterShader("Terrace", "res/shaders/map/terrace.glsl");
    m_HeightEditor.Attach<SliderIntTask>("Terrace", "uNumTerrace", "Number", 1, 10, 4);
    m_HeightEditor.Attach<SliderFloatTask>("Terrace", "uFlatness", "Flatness", 0.0f, 1.0f, 0.0f);
    m_HeightEditor.Attach<SliderFloatTask>("Terrace", "uStrength", "Strength", 0.0f, 1.0f, 0.0f);

    m_HeightEditor.RegisterShader("Radial cutoff", "res/shaders/map/radial_cutoff.glsl");
    m_HeightEditor.Attach<SliderFloatTask>("Radial cutoff", "uBias", "Bias", 0.0f, 1.0f, 0.5f);
    m_HeightEditor.Attach<SliderFloatTask>("Radial cutoff", "uSlope", "Slope", 0.0f, 10.0f, 4.0f);

    //Initial procedures:
    m_HeightEditor.AddProcedureInstance("Const Value");
    m_HeightEditor.AddProcedureInstance("FBM");
    m_HeightEditor.AddProcedureInstance("Radial cutoff");

    //-----Set update flags
    m_UpdateFlags = Height | Normal | Shadow;

    //-----Mipmap related things

    //Nice utilities:
    //http://www.graphics.stanford.edu/~seander/bithacks.html

    auto isPowerOf2 = [](int value)
    {
        return (value & (value - 1)) == 0;
    };

    auto log2 = [](int value)
    {
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

void MapGenerator::UpdateHeight()
{
    ProfilerGPUEvent we("Map::UpdateHeight");

    const int res = m_Heightmap->getResolutionX();

    m_Heightmap->BindImage(0, 0);
    m_HeightEditor.OnDispatch(res);

    m_Heightmap->Bind();

    GenMaxMips();

    m_ResourceManager.RequestPreviewUpdate(m_Heightmap);
}

void MapGenerator::UpdateNormal()
{
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

void MapGenerator::UpdateShadow(const glm::vec3& sun_dir)
{
    ProfilerGPUEvent we("Map::UpdateShadow");

    const int res = m_Shadowmap->getResolutionX();

    m_Heightmap->Bind();
    m_Shadowmap->BindImage(0, 0);

    m_ShadowmapShader->Bind();
    m_ShadowmapShader->setUniform1i("uResolution", res);
    m_ShadowmapShader->setUniform3f("uSunDir", sun_dir);
    m_ShadowmapShader->setUniform1f("uScaleXZ", m_ScaleXZ);
    m_ShadowmapShader->setUniform1f("uScaleY", m_ScaleY);

    m_ShadowmapShader->setUniform1i("uMipOffset", m_ShadowSettings.MipOffset);
    m_ShadowmapShader->setUniform1i("uMinLvl", m_ShadowSettings.MinLevel);
    m_ShadowmapShader->setUniform1i("uStartCell", m_ShadowSettings.StartCell);
    m_ShadowmapShader->setUniform1f("uNudgeFactor", m_ShadowSettings.NudgeFac);
    m_ShadowmapShader->setUniformBool("uSoftShadows", m_ShadowSettings.Soft);
    m_ShadowmapShader->setUniform1f("uSharpness", m_ShadowSettings.Sharpness);

    m_ShadowmapShader->Dispatch(res, res, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_Shadowmap->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    m_ResourceManager.RequestPreviewUpdate(m_Shadowmap);
}


void MapGenerator::GenMaxMips()
{
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

void MapGenerator::Update(const glm::vec3& sun_dir)
{
    if ((m_UpdateFlags & Height) != None)
        UpdateHeight();

    if ((m_UpdateFlags & Normal) != None)
        UpdateNormal();

    if ((m_UpdateFlags & Shadow) != None)
        UpdateShadow(sun_dir);

    m_UpdateFlags = None;
}

void MapGenerator::BindHeightmap(int id) const
{
    m_Heightmap->Bind(id);
}

void MapGenerator::BindNormalmap(int id) const
{
    m_Normalmap->Bind(id);
}

void MapGenerator::BindShadowmap(int id) const
{
    m_Shadowmap->Bind(id);
}

void MapGenerator::ImGuiTerrain(bool &open, bool update_shadows)
{
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

    if (height_changed)
    {
        m_UpdateFlags = m_UpdateFlags | Height | Normal;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }

    else if (scale_changed)
    {
        m_ScaleXZ = scale_xz;
        m_ScaleY = scale_y;

        m_UpdateFlags = m_UpdateFlags | Normal;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }

}

void MapGenerator::ImGuiShadowmap(bool &open, bool update_shadows)
{
    ShadowmapSettings temp = m_ShadowSettings;
    AOSettings temp2 = m_AOSettings;

    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOFI_ICONS_SHADOW "Shadow/AO settings", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    ImGuiUtils::BeginGroupPanel("Shadowmap settings:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderInt("Min level", &temp.MinLevel, 0, 12);
    ImGuiUtils::ColSliderFloat("Nudge fac", &temp.NudgeFac, 1.005f, 1.1f);
    ImGuiUtils::ColCheckbox("Soft Shadows", &temp.Soft);
    ImGuiUtils::ColSliderFloat("Sharpness", &temp.Sharpness, 0.1f, 3.0f);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGuiUtils::BeginGroupPanel("AO settings:");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderInt("AO Samples", &temp2.Samples, 1, 64);
    ImGuiUtils::ColSliderFloat("AO Radius", &temp2.R, 0.0f, 0.1f);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGui::End();

    if (temp2 != m_AOSettings)
    {
        m_AOSettings = temp2;
        m_UpdateFlags = m_UpdateFlags | Normal;
    }

    if (temp != m_ShadowSettings)
    {
        temp.StartCell = static_cast<int>(std::pow(2, temp.MinLevel));

        m_ShadowSettings = temp;

        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | Shadow;
    }
}

void MapGenerator::RequestShadowUpdate() const
{
    m_UpdateFlags = m_UpdateFlags | Shadow;
}

bool MapGenerator::GeometryShouldUpdate()
{
    //If height changed, then normal must also change, but it is possible
    //to change normals without height by changing the scale
    return (m_UpdateFlags & Normal) != None;
}

void MapGenerator::OnSerialize(nlohmann::ordered_json& output)
{
    output["Scale XZ"] = m_ScaleXZ;
    output["Scale Y"] = m_ScaleY;

    m_HeightEditor.OnSerialize(output);
}

void MapGenerator::OnDeserialize(nlohmann::ordered_json& input)
{
    m_ScaleXZ = input["Scale XZ"];
    m_ScaleY = input["Scale Y"];

    m_HeightEditor.OnDeserialize(input[m_HeightEditor.getName()]);

    m_UpdateFlags = Height | Normal | Shadow;
}

//Settings structs operator overloads:

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs)
{
    return (lhs.MinLevel == rhs.MinLevel) && (lhs.StartCell == rhs.StartCell)
        && (lhs.NudgeFac == rhs.NudgeFac) && (lhs.Soft == rhs.Soft)
        && (lhs.Sharpness == rhs.Sharpness);
}

bool operator!=(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs)
{
    return !(lhs==rhs);
}

bool operator==(const AOSettings& lhs, const AOSettings& rhs)
{
    return (lhs.Samples == rhs.Samples) && (lhs.R == rhs.R);
}

bool operator!=(const AOSettings& lhs, const AOSettings& rhs)
{
    return !(lhs==rhs);
}
