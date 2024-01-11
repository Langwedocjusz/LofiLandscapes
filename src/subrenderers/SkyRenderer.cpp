#include "SkyRenderer.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include "Profiler.h"

SkyRenderer::SkyRenderer(ResourceManager& manager, const PerspectiveCamera& cam, const MapGenerator& map)
    : m_ResourceManager(manager)
    , m_Camera(cam)
    , m_Map(map)
{
    m_TransShader       = m_ResourceManager.RequestComputeShader("res/shaders/sky/transmittance.glsl");
    m_MultiShader       = m_ResourceManager.RequestComputeShader("res/shaders/sky/multiscatter.glsl");
    m_SkyShader         = m_ResourceManager.RequestComputeShader("res/shaders/sky/skyview.glsl");
    m_IrradianceShader  = m_ResourceManager.RequestComputeShader("res/shaders/sky/irradiance.glsl");
    m_PrefilteredShader = m_ResourceManager.RequestComputeShader("res/shaders/sky/prefiltered.glsl");
    m_FinalShader       = m_ResourceManager.RequestVertFragShader("res/shaders/sky/final.vert", "res/shaders/sky/final.frag");

    if (!m_AerialShadows)
    {
        m_AerialShader = m_ResourceManager.RequestComputeShader("res/shaders/sky/aerial.glsl");
    }

    else
    {
        m_AScatterShader = m_ResourceManager.RequestComputeShader("res/shaders/sky/scatter_volume.glsl");
        m_AShadowShader = m_ResourceManager.RequestComputeShader("res/shaders/sky/shadow_volume.glsl");
        m_ARaymarchShader = m_ResourceManager.RequestComputeShader("res/shaders/sky/aerial_shadowed.glsl");
    }

    m_TransLUT       = m_ResourceManager.RequestTexture2D();
    m_MultiLUT       = m_ResourceManager.RequestTexture2D();
    m_SkyLUT         = m_ResourceManager.RequestTexture2D();
    m_IrradianceMap  = m_ResourceManager.RequestCubemap();
    m_PrefilteredMap = m_ResourceManager.RequestCubemap();

    m_AerialLUT = m_ResourceManager.RequestTexture3D();

    if (m_AerialShadows)
    {
        m_ScatterVolume = m_ResourceManager.RequestTexture3D();
        m_ShadowVolume = m_ResourceManager.RequestTexture3D();
    }

    Init();
}

void SkyRenderer::Init() {
    //Resolutions
    const int trans_res = 256, multi_res = 32, sky_res = 128; //Regular square
    const int irr_res = 32, pref_res = 128; //Cubemap
    const int aerial_res = 32; //3d

    //Initialize LUT textures

    m_TransLUT->Initialize(Texture2DSpec{
        trans_res, trans_res, GL_RGBA16, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    });

    m_MultiLUT->Initialize(Texture2DSpec{
        multi_res, multi_res, GL_RGBA16, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    });

    m_SkyLUT->Initialize(Texture2DSpec{
        sky_res, sky_res, GL_RGBA16, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_MIRRORED_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    });

    //Initialize Aerial LUT

    if (!m_AerialShadows)
    {
        m_AerialLUT->Initialize(Texture3DSpec{
            32, 32, 32,
            GL_RGBA16, GL_RGBA,
            GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            {0.0f, 0.0f, 0.0f, 0.0f}
        });
    }

    else
    {
        const uint32_t res_x = 160;
        const uint32_t res_y = 90;
        const uint32_t res_z = 64;

        m_AerialLUT->Initialize(Texture3DSpec{
            res_x, res_y, res_z,
            GL_RGBA16, GL_RGBA,
            GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            {0.0f, 0.0f, 0.0f, 0.0f}
        });

        m_ScatterVolume->Initialize(Texture3DSpec{
            res_x, res_y, res_z,
            GL_RGBA16, GL_RGBA,
            GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            {0.0f, 0.0f, 0.0f, 0.0f}
        });

        m_ShadowVolume->Initialize(Texture3DSpec{
            res_x, res_y, res_z,
            GL_R16F, GL_RED,
            GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            {0.0f, 0.0f, 0.0f, 0.0f}
        });
    }

    //Initialize Cubemaps
    m_IrradianceMap->Initialize(CubemapSpec{
        irr_res, GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR,
    });

    m_PrefilteredMap->Initialize(CubemapSpec{
        pref_res, GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
    });

    m_PrefilteredMap->Bind();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //Initialize sun direction
    float cT = cos(m_Theta), sT = sin(m_Theta);
    float cP = cos(m_Phi), sP = sin(m_Phi);
    m_SunDir = glm::vec3(cP * sT, cT, sP * sT);

    //Draw all LUTs & cubemap
    m_UpdateFlags = Transmittance;
}

void SkyRenderer::Update(bool aerial) {

    if ((m_UpdateFlags & Transmittance) != None)
    {
        UpdateTrans();
        UpdateMulti();
        UpdateSky();
    }
    else if ((m_UpdateFlags & MultiScatter) != None)
    {
        UpdateMulti();
        UpdateSky();
    }
    else if ((m_UpdateFlags & SkyView) != None)
    {
        UpdateSky();
    }

    if (aerial)
    {
        if (!m_AerialShadows)
            UpdateAerial();
        else
            UpdateAerialWithShadows();
    }

    if ((m_UpdateFlags & SunColor) != None)
        CalculateSunTransmittance();

    m_UpdateFlags = None;
    m_SunDirChanged = false;
}

void SkyRenderer::UpdateTrans() {
    ProfilerGPUEvent we("Sky::UpdateTransLUT");

    m_TransLUT->BindImage(0, 0);

    m_TransShader->Bind();

    const int res_x = m_TransLUT->getSpec().ResolutionX;
    const int res_y = m_TransLUT->getSpec().ResolutionY;

    m_TransShader->Dispatch(res_x, res_y, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_TransLUT);
}

void SkyRenderer::UpdateMulti() {
    ProfilerGPUEvent we("Sky::UpdateMultiLUT");

    m_TransLUT->Bind(0);
    m_MultiLUT->BindImage(0, 0);

    m_MultiShader->Bind();
    m_MultiShader->setUniform1i("transLUT", 0);
    m_MultiShader->setUniform3f("uGroundAlbedo", m_GroundAlbedo);

    const int res_x = m_MultiLUT->getSpec().ResolutionX;
    const int res_y = m_MultiLUT->getSpec().ResolutionY;

    m_MultiShader->Dispatch(res_x, res_y, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_MultiLUT);
}

void SkyRenderer::UpdateSky() {
    ProfilerGPUEvent we("Sky::UpdateSkyLUT");

    //Update sky lut
    m_TransLUT->Bind(0);
    m_MultiLUT->Bind(1);
    m_SkyLUT->BindImage(0, 0);

    m_SkyShader->Bind();
    m_SkyShader->setUniform1i("transLUT", 0);
    m_SkyShader->setUniform1i("multiLUT", 1);
    m_SkyShader->setUniform3f("uSunDir", m_SunDir);
    m_SkyShader->setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter

    const int res_x = m_SkyLUT->getSpec().ResolutionX;
    const int res_y = m_SkyLUT->getSpec().ResolutionY;

    m_SkyShader->Dispatch(res_x, res_y, 1);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_SkyLUT);

    //Update cubemaps
    const int irr_res = m_IrradianceMap->getSpec().Resolution;

    m_IrradianceMap->BindImage(0, 0);
    m_SkyLUT->Bind();

    m_IrradianceShader->Bind();
    m_IrradianceShader->setUniform1i("uResolution", m_IrradianceMap->getSpec().Resolution);
    m_IrradianceShader->setUniform3f("uSunDir", m_SunDir);
    m_IrradianceShader->setUniform1f("uSkyBrightness", m_Brightness);
    m_IrradianceShader->setUniform1f("uIBLOversaturation", m_IBLOversaturation);

    m_IrradianceShader->Dispatch(irr_res, irr_res, 6);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_IrradianceMap);

    const int pref_res = m_PrefilteredMap->getSpec().Resolution;

    m_PrefilteredMap->BindImage(0, 0);
    m_SkyLUT->Bind();

    m_PrefilteredShader->Bind();
    m_PrefilteredShader->setUniform1i("uResolution", m_PrefilteredMap->getSpec().Resolution);
    m_PrefilteredShader->setUniform3f("uSunDir", m_SunDir);
    m_PrefilteredShader->setUniform1f("uSkyBrightness", m_Brightness);
    m_PrefilteredShader->setUniform1f("uIBLOversaturation", m_IBLOversaturation);

    m_PrefilteredShader->Dispatch(pref_res, pref_res, 6);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_PrefilteredMap->Bind();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    m_ResourceManager.RequestPreviewUpdate(m_PrefilteredMap);
}

void SkyRenderer::UpdateAerial()
{
    ProfilerGPUEvent we("Sky::UpdateAerial");

    m_AerialLUT->BindImage(0, 0);

    m_TransLUT->Bind(0);
    m_MultiLUT->Bind(1);

    m_AerialShader->Bind();
    m_AerialShader->setUniform1i("transLUT", 0);
    m_AerialShader->setUniform1i("multiLUT", 1);
    m_AerialShader->setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter
    m_AerialShader->setUniform3f("uSunDir", m_SunDir);
    m_AerialShader->setUniform1f("uNear", glm::radians(m_Camera.getNearPlane()));
    m_AerialShader->setUniform1f("uFar", m_Camera.getFarPlane());
    m_AerialShader->setUniform3f("uFront", m_Camera.getFront());

    const FrustumExtents extents = m_Camera.getFrustumExtents();

    m_AerialShader->setUniform3f("uBotLeft", extents.BottomLeft);
    m_AerialShader->setUniform3f("uBotRight", extents.BottomRight);
    m_AerialShader->setUniform3f("uTopLeft", extents.TopLeft);
    m_AerialShader->setUniform3f("uTopRight", extents.TopRight);

    m_AerialShader->setUniform1f("uDistScale", m_AerialDistWrite);
    //m_AerialShader->setUniform3f("uGroundAlbedo", m_GroundAlbedo);
    m_AerialShader->setUniform1i("uMultiscatter", int(m_AerialMultiscatter));
    m_AerialShader->setUniform1f("uMultiWeight", m_AerialMultiWeight);

    m_AerialShader->setUniform1f("uBrightness", m_AerialBrightness);

    const int res_x = m_AerialLUT->getSpec().ResolutionX;
    const int res_y = m_AerialLUT->getSpec().ResolutionY;
    const int res_z = m_AerialLUT->getSpec().ResolutionZ;

    m_AerialShader->Dispatch(res_x, res_y, res_z);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_AerialLUT);
}

void SkyRenderer::UpdateAerialWithShadows()
{
    ProfilerGPUEvent we("Sky::UpdateAerial");

    const FrustumExtents extents = m_Camera.getFrustumExtents();

    //All 3d textures used here have the same resolution by assumption
    int res_x = m_ScatterVolume->getSpec().ResolutionX;
    int res_y = m_ScatterVolume->getSpec().ResolutionY;
    int res_z = m_ScatterVolume->getSpec().ResolutionZ;

    //Update scatter volume
    m_ScatterVolume->BindImage(0, 0);

    m_TransLUT->Bind(0);
    m_MultiLUT->Bind(1);

    m_AScatterShader->Bind();
    m_AScatterShader->setUniform1i("transLUT", 0);
    m_AScatterShader->setUniform1i("multiLUT", 1);
    m_AScatterShader->setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter
    m_AScatterShader->setUniform3f("uSunDir", m_SunDir);
    m_AScatterShader->setUniform1f("uNear", glm::radians(m_Camera.getNearPlane()));
    m_AScatterShader->setUniform1f("uFar", m_Camera.getFarPlane());
    m_AScatterShader->setUniform3f("uFront", m_Camera.getFront());
    m_AScatterShader->setUniform3f("uBotLeft", extents.BottomLeft);
    m_AScatterShader->setUniform3f("uBotRight", extents.BottomRight);
    m_AScatterShader->setUniform3f("uTopLeft", extents.TopLeft);
    m_AScatterShader->setUniform3f("uTopRight", extents.TopRight);
    m_AScatterShader->setUniform1f("uDistScale", m_AerialDistWrite);
    //m_AerialShader->setUniform3f("uGroundAlbedo", m_GroundAlbedo);
    m_AScatterShader->setUniform1i("uMultiscatter", int(m_AerialMultiscatter));
    m_AScatterShader->setUniform1f("uMultiWeight", m_AerialMultiWeight);

    m_AScatterShader->Dispatch(res_x, res_y, res_z);

    //No barrier here, since we want them to run in parallel
    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    //Update shadow volume
    m_ShadowVolume->BindImage(1, 0);

    m_Map.BindShadowmap(2);

    m_AShadowShader->Bind();
    m_AShadowShader->setUniform1i("shadowmap", 2);
    m_AShadowShader->setUniform3f("uPos", m_Camera.getPos());
    m_AShadowShader->setUniform3f("uSunDir", m_SunDir);
    m_AShadowShader->setUniform1f("uNear", glm::radians(m_Camera.getNearPlane()));
    m_AShadowShader->setUniform1f("uFar", m_Camera.getFarPlane());
    m_AShadowShader->setUniform3f("uFront", m_Camera.getFront());
    m_AShadowShader->setUniform3f("uBotLeft", extents.BottomLeft);
    m_AShadowShader->setUniform3f("uBotRight", extents.BottomRight);
    m_AShadowShader->setUniform3f("uTopLeft", extents.TopLeft);
    m_AShadowShader->setUniform3f("uTopRight", extents.TopRight);
    m_AShadowShader->setUniform1f("uScaleY", m_Map.getScaleY());
    m_AShadowShader->setUniform1f("uScaleXZ", m_Map.getScaleXZ());

    m_AShadowShader->Dispatch(res_x, res_y, res_z);
    
    //We want to use result of the previous two, so we need a barrier here
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    //Update final AerialLUT
    m_AerialLUT->BindImage(0, 0);

    m_ScatterVolume->Bind(0);
    m_ShadowVolume->Bind(1);

    m_ARaymarchShader->Bind();
    m_ARaymarchShader->setUniform1i("scatterVolume", 0);
    m_ARaymarchShader->setUniform1i("shadowVolume", 1);

    m_ARaymarchShader->setUniform1f("uBrightness", m_AerialBrightness);
    m_ARaymarchShader->setUniform1i("uShadows", m_ShowShadows);

    m_ARaymarchShader->Dispatch(res_x, res_y, res_z);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

//Draws sky on a fullscreen quad, meant to be called after rendering scene geometry
void SkyRenderer::Render() {
    ProfilerGPUEvent we("Sky::Render");

    const FrustumExtents extents = m_Camera.getFrustumExtents();

    m_TransLUT->Bind(0);
    m_SkyLUT->Bind(1);

    m_FinalShader->Bind();
    m_FinalShader->setUniform1i("transLUT", 0);
    m_FinalShader->setUniform1i("skyLUT", 1);
    m_FinalShader->setUniform3f("uSunDir", m_SunDir);

    m_FinalShader->setUniform3f("uBotLeft", extents.BottomLeft);
    m_FinalShader->setUniform3f("uBotRight", extents.BottomRight);
    m_FinalShader->setUniform3f("uTopLeft", extents.TopLeft);
    m_FinalShader->setUniform3f("uTopRight", extents.TopRight);
    m_FinalShader->setUniform1f("uSkyBrightness", m_Brightness);
    m_FinalShader->setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter

    m_Quad.Draw();
}

void SkyRenderer::OnImGui(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOFI_ICONS_SKY "Sky settings", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    const glm::vec3 sun_col = m_SunCol;
    const bool use_trans = m_UseSunTransmittance;
    const float trans_inf = m_TransInfluence;
    const float trans_curve = m_TransCurve;

    ImGuiUtils::BeginGroupPanel("Sun color");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColColorEdit3("Base Color", &m_SunCol);
    ImGuiUtils::ColCheckbox("Use transmittance", &m_UseSunTransmittance);
    ImGuiUtils::ColSliderFloat("Transmittance Influence", &m_TransInfluence, 0.0f, 1.0f);
    ImGuiUtils::ColSliderFloat("Transmittance Curve", &m_TransCurve, 0.0f, 5.0f);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    if (sun_col != m_SunCol || use_trans != m_UseSunTransmittance || trans_inf != m_TransInfluence || trans_curve != m_TransCurve)
        m_UpdateFlags = m_UpdateFlags | SunColor;

    float phi = m_Phi, theta = m_Theta;
    float height = m_Height;

    ImGuiUtils::BeginGroupPanel("Sun position");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderFloat("Phi", &phi, 0.0, 6.28);
    ImGuiUtils::ColSliderFloat("Theta", &theta, 0.0, 0.5 * 3.14);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGuiUtils::BeginGroupPanel("Planet/Atmosphere");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderFloat("Height (m)", &height, 0.0f, 2000.0f);

    const glm::vec3 sun_dir = m_SunDir;

    if (phi != m_Phi || theta != m_Theta || height != m_Height) {
        m_Phi = phi;
        m_Theta = theta;
        m_Height = height;

        const float cT = cos(theta), sT = sin(theta);
        const float cP = cos(phi),   sP = sin(phi);

        m_SunDir = glm::vec3(cP * sT, cT, sP * sT);

        m_UpdateFlags = m_UpdateFlags | SkyView;
    }

    if (sun_dir != m_SunDir)
    {
        m_SunDirChanged = true;
        m_UpdateFlags = m_UpdateFlags | SunColor;
    }

    glm::vec3 albedo = m_GroundAlbedo;

    ImGuiUtils::ColColorEdit3("Ground Albedo", &albedo);

    if (albedo != m_GroundAlbedo) {
        m_GroundAlbedo = albedo;
        m_UpdateFlags = m_UpdateFlags | SkyView;
        m_UpdateFlags = m_UpdateFlags | MultiScatter;
    }

    ImGuiUtils::ColSliderFloat("IBL Oversaturation", &m_IBLOversaturation, 1.0f, 3.0f);
    ImGuiUtils::ColSliderFloat("Brightness", &m_Brightness, 0.0f, 10.0f);
    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGuiUtils::BeginGroupPanel("Fog");
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderFloat("Aerial brightness", &m_AerialBrightness, 0.0f, 500.0f);
    ImGuiUtils::ColSliderFloat("Dist scale (write)", &m_AerialDistWrite, 0.0f, 10.0f);
    ImGuiUtils::ColSliderFloat("Dist scale (read)", &m_AerialDistRead, 0.0f, 10.0f);
    ImGuiUtils::ColCheckbox("Multiscatter", &m_AerialMultiscatter);
    ImGuiUtils::ColSliderFloat("Multi factor", &m_AerialMultiWeight, 0.0f, 1.0f);

    if (m_AerialShadows)
        ImGuiUtils::ColCheckbox("Shadows", &m_ShowShadows);

    ImGui::Columns(1, "###col");
    ImGuiUtils::EndGroupPanel();

    ImGui::End();
}

void SkyRenderer::BindSkyLUT(int id) const {
    m_SkyLUT->Bind(id);
}

void SkyRenderer::BindIrradiance(int id) const {
    m_IrradianceMap->Bind(id);
}

void SkyRenderer::BindPrefiltered(int id) const {
    m_PrefilteredMap->Bind(id);
}

void SkyRenderer::BindAerial(int id) const {
    m_AerialLUT->Bind(id);
}

glm::vec3 SkyRenderer::getSunCol() const
{
    if (m_UseSunTransmittance)
        return m_SunTrans * m_SunCol;
    else
        return m_SunCol;
}

void SkyRenderer::CalculateSunTransmittance()
{
    //TO-DO:
    //Currently all atmosphere parameters defined here are doubled in shader files
    //Once they are exposed in the gui this will need to be unified

    constexpr int num_steps = 40;

    constexpr float ground_rad = 6.360f;
    constexpr float atmosphere_rad = 6.460f;

    constexpr glm::vec3 pos{ 0.0f, ground_rad, 0.0f };

    //Ray-sphere intersection from
    //https://gamedev.stackexchange.com/questions/96459/fast-ray-sphere-collision-code.
    auto IntersectSphere = [](glm::vec3 ro, glm::vec3 rd, float rad)
    {
        float b = glm::dot(ro, rd);
        float c = glm::dot(ro, ro) - rad * rad;
        if (c > 0.0 && b > 0.0) return -1.0f;
        float discr = b * b - c;
        if (discr < 0.0) return -1.0f;
        // Special case: inside sphere, use far discriminant
        if (discr > b * b) return (-b + glm::sqrt(discr));
        return -b - glm::sqrt(discr);
    };

    auto getScatteringValues = [ground_rad](glm::vec3 pos, glm::vec3& rayleigh_s, float& mie_s, glm::vec3& extinction)
    {
        //Atmosphere params
        constexpr glm::vec3 base_rayleigh_s{ 5.802f, 13.558f, 33.1f };
        constexpr float base_rayleigh_a = 0.0f;

        constexpr float base_mie_s = 3.996f;
        constexpr float base_mie_a = 4.4f;

        constexpr glm::vec3 base_ozone_a{ 0.650f, 1.881f, 0.085f };

        //Height in km
        float altitude = (glm::length(pos) - ground_rad) * 1000.0f;

        //Density(height) distributions
        // Note: Paper gets these switched up.
        float rayleigh_dens = glm::exp(-altitude / 8.0f);
        float mie_dens = glm::exp(-altitude / 1.2f);

        rayleigh_s = base_rayleigh_s * rayleigh_dens;
        float rayleigh_a = base_rayleigh_a * rayleigh_dens;

        mie_s = base_mie_s * mie_dens;
        float mie_a = base_mie_a * mie_dens;

        //Ozone - uniform, triangle distribution
        glm::vec3 ozone_a = base_ozone_a * glm::max(0.0f, 1.0f - abs(altitude - 25.0f) / 15.0f);

        //Extinction is a sum of absorbionts and scatterings
        extinction = rayleigh_s + rayleigh_a + mie_s + mie_a + ozone_a;
    };

    //Ray is hitting the Earth - no transmittance
    if (IntersectSphere(pos, m_SunDir, ground_rad) > 0.0f)
    {
        m_SunTrans = glm::vec3(0.0f);
        return;
    }

    //Distance to edge of the atmosphere
    const float atm_dist = IntersectSphere(pos, m_SunDir, atmosphere_rad);

    //Integrate transmittance
    const float dt = atm_dist / float(num_steps);

    float t = 0.3 * dt; //starting offset

    glm::vec3 res(1.0f);

    for (int i = 0; i < num_steps; i++) {
        glm::vec3 p = pos + t * m_SunDir;

        glm::vec3 rayleigh_s, extinction;
        float mie_s;

        getScatteringValues(p, rayleigh_s, mie_s, extinction);

        //Beer-Lambert law
        res *= glm::exp(-dt * extinction);

        t += dt;
    }

    auto InterpolateResult = [this](glm::vec3 res)
    {
        const float h = glm::max(1.0f - m_SunDir.y, 0.0f);
        const float t = m_TransInfluence * glm::pow(h, m_TransCurve);

        return t * res + (1.0f - t) * glm::vec3(1.0f);
    };

    m_SunTrans = InterpolateResult(res);
}