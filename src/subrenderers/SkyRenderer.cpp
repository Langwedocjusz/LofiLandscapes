#include "SkyRenderer.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include "Profiler.h"

SkyRenderer::SkyRenderer(ResourceManager& manager)
    : m_ResourceManager(manager)
{
    m_TransShader       = m_ResourceManager.RequestComputeShader("res/shaders/sky/transmittance.glsl");
    m_MultiShader       = m_ResourceManager.RequestComputeShader("res/shaders/sky/multiscatter.glsl");
    m_SkyShader         = m_ResourceManager.RequestComputeShader("res/shaders/sky/skyview.glsl");
    m_IrradianceShader  = m_ResourceManager.RequestComputeShader("res/shaders/sky/irradiance.glsl");
    m_PrefilteredShader = m_ResourceManager.RequestComputeShader("res/shaders/sky/prefiltered.glsl");
    m_AerialShader      = m_ResourceManager.RequestComputeShader("res/shaders/sky/aerial.glsl");
    m_FinalShader       = m_ResourceManager.RequestVertFragShader("res/shaders/sky/final.vert", "res/shaders/sky/final.frag");

    m_TransLUT       = m_ResourceManager.RequestTexture2D();
    m_MultiLUT       = m_ResourceManager.RequestTexture2D();
    m_SkyLUT         = m_ResourceManager.RequestTexture2D();
    m_AerialLUT      = m_ResourceManager.RequestTexture3D();
    m_IrradianceMap  = m_ResourceManager.RequestCubemap();
    m_PrefilteredMap = m_ResourceManager.RequestCubemap();

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
    m_AerialLUT->Initialize(Texture3DSpec{
        aerial_res, aerial_res, aerial_res,
        GL_RGBA16, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_CLAMP_TO_EDGE,
        {0.0f, 0.0f, 0.0f, 0.0f}
    });

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

void SkyRenderer::Update(const Camera& cam, float aspect, bool aerial) {

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
        UpdateAerial(cam, aspect);

    m_UpdateFlags = None;
}

void SkyRenderer::UpdateTrans() {
    ProfilerGPUEvent we("Sky::UpdateTransLUT");

    const int res = m_TransLUT->getSpec().ResolutionX;

    m_TransLUT->BindImage(0, 0);

    m_TransShader->Bind();
    m_TransShader->setUniform1i("uResolution", res);

    m_TransShader->Dispatch(res, res, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_TransLUT);
}

void SkyRenderer::UpdateMulti() {
    ProfilerGPUEvent we("Sky::UpdateMultiLUT");

    const int res = m_MultiLUT->getSpec().ResolutionX;

    m_TransLUT->Bind(0);
    m_MultiLUT->BindImage(0, 0);

    m_MultiShader->Bind();
    m_MultiShader->setUniform1i("transLUT", 0);
    m_MultiShader->setUniform1i("uResolution", res);
    m_MultiShader->setUniform3f("uGroundAlbedo", m_GroundAlbedo);

    m_MultiShader->Dispatch(res, res, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_MultiLUT);
}

void SkyRenderer::UpdateSky() {
    ProfilerGPUEvent we("Sky::UpdateSkyLUT");

    //Update sky lut
    const int res = m_SkyLUT->getSpec().ResolutionX;

    m_TransLUT->Bind(0);
    m_MultiLUT->Bind(1);
    m_SkyLUT->BindImage(0, 0);

    m_SkyShader->Bind();
    m_SkyShader->setUniform1i("transLUT", 0);
    m_SkyShader->setUniform1i("multiLUT", 1);
    m_SkyShader->setUniform1i("uResolution", res);
    m_SkyShader->setUniform3f("uSunDir", m_SunDir);
    m_SkyShader->setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter

    m_SkyShader->Dispatch(res, res, 1);
    
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

void SkyRenderer::UpdateAerial(const Camera& cam, float aspect)
{
    ProfilerGPUEvent we("Sky::UpdateAerial");

    const int res_x = m_AerialLUT->getSpec().ResolutionX;
    const int res_y = m_AerialLUT->getSpec().ResolutionY;
    const int res_z = m_AerialLUT->getSpec().ResolutionZ;

    m_AerialLUT->BindImage(0, 0);

    m_AerialShader->Bind();
    m_AerialShader->setUniform1i("transLUT", 0);
    m_AerialShader->setUniform1i("multiLUT", 1);
    m_AerialShader->setUniform1i("uResolution", res_z);
    m_AerialShader->setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter
    m_AerialShader->setUniform3f("uSunDir", m_SunDir);
    m_AerialShader->setUniform1f("uFar", cam.getFarPlane());
    m_AerialShader->setUniform1f("uFov", glm::radians(cam.getFov()));
    m_AerialShader->setUniform1f("uAspect", aspect);
    m_AerialShader->setUniform3f("uFront", cam.getFront());
    m_AerialShader->setUniform3f("uRight", cam.getRight());
    m_AerialShader->setUniform3f("uTop", cam.getUp());
    m_AerialShader->setUniform1f("uBrightness", m_AerialBrightness);
    m_AerialShader->setUniform1f("uDistScale", m_AerialDistscale);

    m_AerialShader->Dispatch(res_x, res_y, res_z);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_ResourceManager.RequestPreviewUpdate(m_AerialLUT);
}

//Draws sky on a fullscreen quad, meant to be called after rendering scene geometry
void SkyRenderer::Render(glm::vec3 cam_dir, float cam_fov, float aspect) {
    ProfilerGPUEvent we("Sky::Render");

    m_TransLUT->Bind(0);
    m_SkyLUT->Bind(1);

    m_FinalShader->Bind();
    m_FinalShader->setUniform1i("transLUT", 0);
    m_FinalShader->setUniform1i("skyLUT", 1);
    m_FinalShader->setUniform3f("uSunDir", m_SunDir);
    m_FinalShader->setUniform3f("uCamDir", cam_dir);
    m_FinalShader->setUniform1f("uCamFov", glm::radians(cam_fov));
    m_FinalShader->setUniform1f("uAspectRatio", aspect);
    m_FinalShader->setUniform1f("uSkyBrightness", m_Brightness);
    m_FinalShader->setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter

    m_Quad.Draw();
}

void SkyRenderer::OnImGui(bool& open) {
    ImGui::Begin("Sky settings", &open, ImGuiWindowFlags_NoFocusOnAppearing);

    float phi = m_Phi, theta = m_Theta;
    float height = m_Height;

    ImGui::Columns(2, "###col");

    ImGuiUtils::SliderFloat("Phi", &phi, 0.0, 6.28);
    ImGuiUtils::SliderFloat("Theta", &theta, 0.0, 0.5 * 3.14);
    ImGuiUtils::SliderFloat("Height (m)", &height, 0.0f, 2000.0f);

    if (phi != m_Phi || theta != m_Theta || height != m_Height) {
        m_Phi = phi;
        m_Theta = theta;
        m_Height = height;

        float cT = cos(theta), sT = sin(theta);
        float cP = cos(phi), sP = sin(phi);

        m_SunDir = glm::vec3(cP * sT, cT, sP * sT);

        m_UpdateFlags = m_UpdateFlags | SkyView;
    }

    glm::vec3 albedo = m_GroundAlbedo;

    ImGuiUtils::ColorEdit3("Ground Albedo", &albedo);

    if (albedo != m_GroundAlbedo) {
        m_GroundAlbedo = albedo;
        m_UpdateFlags = m_UpdateFlags | SkyView;
        m_UpdateFlags = m_UpdateFlags | MultiScatter;
    }

    ImGuiUtils::SliderFloat("IBL Oversaturation", &m_IBLOversaturation, 1.0f, 3.0f);

    ImGuiUtils::SliderFloat("Brightness", &m_Brightness, 0.0f, 10.0f);

    ImGuiUtils::SliderFloat("Aerial brightness", &m_AerialBrightness, 0.0f, 500.0f);
    ImGuiUtils::SliderFloat("Aerial distance", &m_AerialDistscale, 0.0f, 10.0f);

    ImGui::Columns(1, "###col");

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