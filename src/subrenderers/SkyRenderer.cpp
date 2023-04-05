#include "SkyRenderer.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

SkyRenderer::SkyRenderer()
    : m_TransShader("res/shaders/sky/transmittance.glsl")
    , m_MultiShader("res/shaders/sky/multiscatter.glsl")
    , m_SkyShader("res/shaders/sky/skyview.glsl")
    , m_IrradianceShader("res/shaders/sky/irradiance.glsl")
    , m_PrefilteredShader("res/shaders/sky/prefiltered.glsl")
    , m_AerialShader("res/shaders/sky/aerial.glsl")
    , m_FinalShader("res/shaders/sky/final.vert", "res/shaders/sky/final.frag")
{
    //Initialize LUT textures
    const int trans_res = 256, multi_res = 32, sky_res = 128;

    TextureSpec trans_spec = TextureSpec{
        trans_res, trans_res, GL_RGBA16, GL_RGBA, 
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    TextureSpec multi_spec = TextureSpec{
        multi_res, multi_res, GL_RGBA16, GL_RGBA, 
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    TextureSpec sky_spec = TextureSpec{
        sky_res, sky_res, GL_RGBA16, GL_RGBA, 
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_MIRRORED_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_TransLUT.Initialize(trans_spec);
    m_MultiLUT.Initialize(multi_spec);
    m_SkyLUT.Initialize(sky_spec);

    //Initialize Aerial LUT (3d)
    const int aerial_res = 32;

    Texture3dSpec aerial_spec = Texture3dSpec{
        aerial_res, aerial_res, aerial_res,
        GL_RGBA16, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_CLAMP_TO_EDGE,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_AerialLUT.Initialize(aerial_spec);

    //Initialize Cubemaps
    const int irr_res = 32, pref_res = 128;

    CubemapSpec irradinace_spec = CubemapSpec{
        irr_res, GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR,
    };

    CubemapSpec prefiltered_spec = CubemapSpec{
        pref_res, GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
    };

    m_IrradianceMap.Initialize(irradinace_spec);
    m_PrefilteredMap.Initialize(prefiltered_spec);

    m_PrefilteredMap.Bind();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //Initialize sun direction
    float cT = cos(m_Theta), sT = sin(m_Theta);
    float cP = cos(m_Phi), sP = sin(m_Phi);
    m_SunDir = glm::vec3(cP * sT, cT, sP * sT);

    //Draw all LUTs & cubemap
    m_UpdateFlags = Transmittance;
}

SkyRenderer::~SkyRenderer() {}

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
    const int res = m_TransLUT.getSpec().ResolutionX;

    m_TransLUT.BindImage(0, 0);

    m_TransShader.Bind();
    m_TransShader.setUniform1i("uResolution", res);

    glDispatchCompute(res / 32, res / 32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SkyRenderer::UpdateMulti() {
    const int res = m_MultiLUT.getSpec().ResolutionX;

    m_TransLUT.Bind(0);
    m_MultiLUT.BindImage(0, 0);

    m_MultiShader.Bind();
    m_MultiShader.setUniform1i("transLUT", 0);
    m_MultiShader.setUniform1i("uResolution", res);
    m_MultiShader.setUniform3f("uGroundAlbedo", m_GroundAlbedo);

    glDispatchCompute(res / 32, res / 32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SkyRenderer::UpdateSky() {
    //Update sky lut
    const int res = m_SkyLUT.getSpec().ResolutionX;

    m_TransLUT.Bind(0);
    m_MultiLUT.Bind(1);
    m_SkyLUT.BindImage(0, 0);

    m_SkyShader.Bind();
    m_SkyShader.setUniform1i("transLUT", 0);
    m_SkyShader.setUniform1i("multiLUT", 1);
    m_SkyShader.setUniform1i("uResolution", res);
    m_SkyShader.setUniform3f("uSunDir", m_SunDir);
    m_SkyShader.setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter

    glDispatchCompute(res / 32, res / 32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    //Update cubemaps
    m_IrradianceMap.BindImage(0, 0);
    m_SkyLUT.Bind();

    m_IrradianceShader.Bind();
    m_IrradianceShader.setUniform1i("uResolution", m_IrradianceMap.getSpec().Resolution);
    m_IrradianceShader.setUniform3f("uSunDir", m_SunDir);
    m_IrradianceShader.setUniform1f("uSkyBrightness", m_Brightness);
    m_IrradianceShader.setUniform1f("uIBLOversaturation", m_IBLOversaturation);

    glDispatchCompute(1, 1, 6);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_PrefilteredMap.BindImage(0, 0);
    m_SkyLUT.Bind();

    m_PrefilteredShader.Bind();
    m_PrefilteredShader.setUniform1i("uResolution", m_PrefilteredMap.getSpec().Resolution);
    m_PrefilteredShader.setUniform3f("uSunDir", m_SunDir);
    m_PrefilteredShader.setUniform1f("uSkyBrightness", m_Brightness);
    m_PrefilteredShader.setUniform1f("uIBLOversaturation", m_IBLOversaturation);

    glDispatchCompute(4, 4, 6);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_PrefilteredMap.Bind();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void SkyRenderer::UpdateAerial(const Camera& cam, float aspect)
{
    const int res = m_AerialLUT.getSpec().ResolutionZ;

    m_AerialLUT.BindImage(0, 0);

    m_AerialShader.Bind();
    m_AerialShader.setUniform1i("transLUT", 0);
    m_AerialShader.setUniform1i("multiLUT", 1);
    m_AerialShader.setUniform1i("uResolution", res);
    m_AerialShader.setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter
    m_AerialShader.setUniform3f("uSunDir", m_SunDir);
    m_AerialShader.setUniform1f("uFar", cam.getFarPlane());
    m_AerialShader.setUniform1f("uFov", glm::radians(cam.getSettings().Fov));
    m_AerialShader.setUniform1f("uAspect", aspect);
    m_AerialShader.setUniform3f("uFront", cam.getFront());
    m_AerialShader.setUniform3f("uRight", cam.getRight());
    m_AerialShader.setUniform3f("uTop", cam.getUp());
    m_AerialShader.setUniform1f("uBrightness", m_AerialBrightness);
    m_AerialShader.setUniform1f("uDistScale", m_AerialDistscale);

    glDispatchCompute(1, 1, res);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

//Draws sky on a fullscreen quad, meant to be called after rendering scene geometry
void SkyRenderer::Render(glm::vec3 cam_dir, float cam_fov, float aspect) {
    m_TransLUT.Bind(0);
    m_SkyLUT.Bind(1);

    m_FinalShader.Bind();
    m_FinalShader.setUniform1i("transLUT", 0);
    m_FinalShader.setUniform1i("skyLUT", 1);
    m_FinalShader.setUniform3f("uSunDir", m_SunDir);
    m_FinalShader.setUniform3f("uCamDir", cam_dir);
    m_FinalShader.setUniform1f("uCamFov", glm::radians(cam_fov));
    m_FinalShader.setUniform1f("uAspectRatio", aspect);
    m_FinalShader.setUniform1f("uSkyBrightness", m_Brightness);
    m_FinalShader.setUniform1f("uHeight", 0.000001f * m_Height); // meter -> megameter

    m_Quad.Draw();
}

void SkyRenderer::OnImGui(bool& open) {
    ImGui::Begin("Sky settings", &open);

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
    m_SkyLUT.Bind(id);
}

void SkyRenderer::BindIrradiance(int id) const {
    m_IrradianceMap.Bind(id);
}

void SkyRenderer::BindPrefiltered(int id) const {
    m_PrefilteredMap.Bind(id);
}

void SkyRenderer::BindAerial(int id) const {
    m_AerialLUT.Bind(id);
}