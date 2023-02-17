#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>

Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height)
    , m_ShadedShader("res/shaders/shaded.vert", 
                     "res/shaders/shaded.frag")
    , m_WireframeShader("res/shaders/wireframe.vert", 
                        "res/shaders/wireframe.frag")
    , m_Aspect(float(m_WindowWidth) / float(m_WindowHeight))
    , m_InvAspect(1.0/m_Aspect)
{}

Renderer::~Renderer() {}

void Renderer::Init(int subdivisions, int levels, int height_res, int shadow_res, int wrap_type) {
    m_Clipmap.Init(subdivisions, levels);
    m_Map.Init(height_res, shadow_res, wrap_type);

    glEnable(GL_DEPTH_TEST);
    //Depth function to allow sky with maximal depth (1.0)
    //being rendered after all geometry
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //Backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    //Seamless cubemaps
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //Update Maps
    m_Map.Update(m_Sky.getSunDir());

    //Update Material
    m_Material.Update();

    //Initial geometry displacement
    m_Map.BindHeightmap();

    glm::vec2 pos{ m_Camera.getPos().x, m_Camera.getPos().z };

    m_Clipmap.DisplaceVertices(
        m_Map.getScaleSettings().ScaleXZ,
        m_Map.getScaleSettings().ScaleY,
        pos
    );

    //Return to normal rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Renderer::OnUpdate(float deltatime) {
    //Update camera
    glm::vec3 prev_pos3 = m_Camera.getPos();
    glm::vec2 prev_pos2 = { prev_pos3.x, prev_pos3.z };

    m_Camera.Update(deltatime);

    glm::vec3 curr_pos3 = m_Camera.getPos();
    glm::vec2 curr_pos2 = { curr_pos3.x, curr_pos3.z };

    glm::mat4 proj = m_Camera.getProjMatrix(m_Aspect);
    glm::mat4 view = m_Camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_MVP = proj * view * model;

    //Update sky
    m_Sky.Update();

    //Update clipmap geometry
    m_Map.BindHeightmap();

    m_Clipmap.DisplaceVertices(
        m_Map.getScaleSettings().ScaleXZ,
        m_Map.getScaleSettings().ScaleY,
        curr_pos2, prev_pos2
    );
}

void Renderer::OnRender() {
    const float scale_xz = m_Map.getScaleSettings().ScaleXZ;
    const float scale_y  = m_Map.getScaleSettings().ScaleY;

    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_Wireframe) {
        m_WireframeShader.Bind();
        m_WireframeShader.setUniform1f("uL", m_Map.getScaleSettings().ScaleXZ);
        m_WireframeShader.setUniform2f("uPos", m_Camera.getPos().x, 
                                               m_Camera.getPos().z);
        m_WireframeShader.setUniformMatrix4fv("uMVP", m_MVP);

        m_Clipmap.BindAndDraw(m_Camera, m_Aspect, scale_y);
    }

    else {
        //Render Clipmap (Terrain)

        m_ShadedShader.Bind();
        m_ShadedShader.setUniform1f("uL", scale_xz);
        m_ShadedShader.setUniform3f("uLightDir", m_Sky.getSunDir());
        m_ShadedShader.setUniform3f("uPos", m_Camera.getPos());
        m_ShadedShader.setUniformMatrix4fv("uMVP", m_MVP);
        m_ShadedShader.setUniform1i("uShadow", int(m_Shadows));
        m_ShadedShader.setUniform1i("uMaterial", int(m_Materials));
        m_ShadedShader.setUniform1i("uFixTiling", int(m_FixTiling));
        m_ShadedShader.setUniform3f("uSunCol", m_SunCol);
        m_ShadedShader.setUniform1f("uSunStr", m_SunStr);
        m_ShadedShader.setUniform1f("uSkyDiff", m_SkyDiff);
        m_ShadedShader.setUniform1f("uSkySpec", m_SkySpec);
        m_ShadedShader.setUniform1f("uRefStr", m_RefStr);
        m_ShadedShader.setUniform1f("uTilingFactor", m_TilingFactor);
        m_ShadedShader.setUniform1f("uNormalStrength", m_NormalStrength);

        m_Map.BindNormalmap(0);
        m_ShadedShader.setUniform1i("normalmap", 0);
        m_Map.BindShadowmap(1);
        m_ShadedShader.setUniform1i("shadowmap", 1);
        m_Material.BindAlbedo(2);
        m_ShadedShader.setUniform1i("albedo", 2);
        m_Material.BindNormal(3);
        m_ShadedShader.setUniform1i("normal", 3);

        m_Sky.BindIrradiance(4);
        m_ShadedShader.setUniform1i("irradiance", 4);
        m_Sky.BindPrefiltered(5);
        m_ShadedShader.setUniform1i("prefiltered", 5);

        m_Clipmap.BindAndDraw(m_Camera, m_Aspect, scale_y);

        //Render Sky
        m_Sky.Render(m_Camera.getFront(), m_Camera.getSettings().Fov, m_InvAspect);
    }
}

void Renderer::OnImGuiRender() {
    //-----Menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Shaded")) {
                m_Wireframe = false;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            if (ImGui::MenuItem("Wireframe")) {
                m_Wireframe = true;
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            if (ImGui::MenuItem("Terrain"))
                m_ShowTerrainMenu = !m_ShowTerrainMenu;

            if (ImGui::MenuItem("Background"))
                m_ShowBackgroundMenu = !m_ShowBackgroundMenu;

            if (ImGui::MenuItem("Lighting"))
                m_ShowLightMenu = !m_ShowLightMenu;

            if (ImGui::MenuItem("Shadows"))
                m_ShowShadowMenu = !m_ShowShadowMenu;

            if (ImGui::MenuItem("Camera"))
                m_ShowCamMenu = !m_ShowCamMenu;

            if (ImGui::MenuItem("Material"))
                m_ShowMaterialMenu = !m_ShowMaterialMenu;

            if (ImGui::MenuItem("Sky"))
                m_ShowSkyMenu = !m_ShowSkyMenu;

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    //-----Dockspace
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

    //-----Windows
    if (m_ShowBackgroundMenu) {
        ImGui::Begin("Background", &m_ShowBackgroundMenu);
        ImGuiUtils::ColorEdit3("ClearColor", m_ClearColor);
        ImGui::End();
    }
    
    if (m_ShowCamMenu) {
        CameraSettings temp = m_Camera.getSettings();

        ImGui::Begin("Camera", &m_ShowCamMenu);
        ImGuiUtils::SliderFloat("Speed", &(temp.Speed), 0.0, 10.0f);
        ImGuiUtils::SliderFloat("Sensitivity", &(temp.Sensitivity), 0.0f, 200.0f);
        ImGuiUtils::SliderFloat("Fov", &(temp.Fov), 0.0f, 90.0f);
        ImGui::End();

        m_Camera.setSettings(temp);
    }
    
    if (m_ShowTerrainMenu)
        m_Map.ImGuiTerrain(m_ShowTerrainMenu, m_Shadows);

    bool update_geo = m_Map.GeometryShouldUpdate();

    if(m_ShowShadowMenu)
        m_Map.ImGuiShadowmap(m_ShowShadowMenu, m_Shadows);

    if (m_ShowMaterialMenu)
        m_Material.OnImGui(m_ShowMaterialMenu);

    if (m_ShowLightMenu) {
        bool shadows = m_Shadows;

        ImGui::Begin("Lighting", &m_ShowLightMenu);
        ImGuiUtils::Checkbox("Shadows", &shadows);
        ImGuiUtils::ColorEdit3("Sun Color", m_SunCol);
        ImGui::Text("Brightness values");
        ImGuiUtils::SliderFloat("Sun" , &m_SunStr, 0.0, 4.0);
        ImGuiUtils::SliderFloat("Sky Diffuse" , &m_SkyDiff, 0.0, 1.0);
        ImGuiUtils::SliderFloat("Sky Specular", &m_SkySpec, 0.0, 1.0);
        ImGuiUtils::SliderFloat("Reflected", &m_RefStr, 0.0, 1.0);
        ImGui::Text("Material params:");
        ImGuiUtils::Checkbox("Materials", &m_Materials);
        ImGuiUtils::Checkbox("Fix Tiling", &m_FixTiling);
        ImGuiUtils::SliderFloat("Tiling Factor", &m_TilingFactor, 0.0, 128.0);
        ImGuiUtils::SliderFloat("Normal Strength", &m_NormalStrength, 0.0, 1.0);
        ImGui::End();

        if (shadows != m_Shadows) {
            m_Shadows = shadows;
            if (m_Shadows) m_Map.RequestShadowUpdate();
        }

        //if (ImGui::Button("Print Frustum"))
        //    m_Clipmap.PrintFrustum(m_Camera, m_Aspect);
    }

    if (m_ShowSkyMenu) {
        glm::vec3 sun_dir = m_Sky.getSunDir();

        m_Sky.OnImGui(m_ShowSkyMenu);

        if (sun_dir != m_Sky.getSunDir() && m_Shadows)
            m_Map.RequestShadowUpdate();
    }

    //Update Maps
    m_Map.Update(m_Sky.getSunDir());

    //Update geometry
    if (update_geo) {
        m_Map.BindHeightmap();

        glm::vec2 pos{ m_Camera.getPos().x, m_Camera.getPos().z };

        m_Clipmap.DisplaceVertices(
            m_Map.getScaleSettings().ScaleXZ,
            m_Map.getScaleSettings().ScaleY,
            pos
        );
    }  
}

void Renderer::OnWindowResize(unsigned int width, unsigned int height) {
    m_WindowWidth = width;
    m_WindowHeight = height;

    m_Aspect = float(m_WindowWidth) / float(m_WindowHeight);
    m_InvAspect = 1.0 / m_Aspect;

    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Renderer::OnKeyPressed(int keycode, bool repeat) {
    m_Camera.OnKeyPressed(keycode, repeat);
}

void Renderer::OnKeyReleased(int keycode) {
    m_Camera.OnKeyReleased(keycode);
}

void Renderer::OnMouseMoved(float x, float y) {
    m_Camera.OnMouseMoved(x, y, m_WindowWidth, m_WindowHeight);
}

void Renderer::RestartMouse() {
    m_Camera.setMouseInit(true);
}
