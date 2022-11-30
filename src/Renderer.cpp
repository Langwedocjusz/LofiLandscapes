#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>


Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height),
      m_ShadedShader("res/shaders/shaded.vert", 
                     "res/shaders/shaded.frag"),
      m_WireframeShader("res/shaders/wireframe.vert", 
                        "res/shaders/wireframe.frag"),
      m_Clipmap(),
      m_Map()
{
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //Backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    //Update Maps
    m_Map.Update(m_Theta, m_Phi);

    //Update Material
    m_Material.Update();

    //Return to normal rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

Renderer::~Renderer() {}

void Renderer::OnUpdate(float deltatime) {
    m_Camera.Update(deltatime);

    glm::mat4 proj = m_Camera.getProjMatrix(m_WindowWidth, m_WindowHeight);
    glm::mat4 view = m_Camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_MVP = proj * view * model;
}

void Renderer::OnRender() {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_Map.BindHeightmap();
    m_Clipmap.DisplaceVertices(m_Map.getScaleSettings().ScaleXZ, 
                               m_Map.getScaleSettings().ScaleY,
                               m_Camera.getPos().x, m_Camera.getPos().z);

    if (m_Wireframe) {
        m_WireframeShader.Bind();
        m_WireframeShader.setUniform1f("uL", m_Map.getScaleSettings().ScaleXZ);
        m_WireframeShader.setUniform2f("uPos", m_Camera.getPos().x, 
                                               m_Camera.getPos().z);
        m_WireframeShader.setUniformMatrix4fv("uMVP", m_MVP);

    }

    else {
        m_ShadedShader.Bind();
        m_ShadedShader.setUniform1f("uL", m_Map.getScaleSettings().ScaleXZ);
        m_ShadedShader.setUniform1f("uTheta", m_Theta);
        m_ShadedShader.setUniform1f("uPhi", m_Phi);
        m_ShadedShader.setUniform3f("uPos", m_Camera.getPos());
        m_ShadedShader.setUniformMatrix4fv("uMVP", m_MVP);
        m_ShadedShader.setUniform1i("uShadow", int(m_Shadows));
        m_ShadedShader.setUniform1i("uMaterial", int(m_Materials));
        m_ShadedShader.setUniform1i("uFixTiling", int(m_FixTiling));
        m_ShadedShader.setUniform4f("uSunCol", m_SunCol);
        m_ShadedShader.setUniform4f("uSkyCol", m_SkyCol);
        m_ShadedShader.setUniform4f("uRefCol", m_RefCol);
        m_ShadedShader.setUniform1f("uTilingFactor", m_TilingFactor);
        m_ShadedShader.setUniform1f("uNormalStrength", m_NormalStrength);
        m_ShadedShader.setUniform1f("uMinSkylight", m_MinSkylight);

        m_Map.BindNormalmap(0);
        m_ShadedShader.setUniform1i("normalmap", 0);
        m_Map.BindShadowmap(1);
        m_ShadedShader.setUniform1i("shadowmap", 1);
        m_Material.BindAlbedo(2);
        m_ShadedShader.setUniform1i("albedo", 2);
        m_Material.BindNormal(3);
        m_ShadedShader.setUniform1i("normal", 3);
    }

    m_Clipmap.BindAndDraw();
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

    if(m_ShowShadowMenu)
        m_Map.ImGuiShadowmap(m_ShowShadowMenu, m_Shadows);

    if (m_ShowMaterialMenu)
        m_Material.OnImGui(m_ShowMaterialMenu);

    if (m_ShowLightMenu) {
        float phi = m_Phi, theta = m_Theta;
        bool shadows = m_Shadows;

        ImGui::Begin("Lighting", &m_ShowLightMenu);
        ImGuiUtils::Checkbox("Shadows", &shadows);
        ImGuiUtils::SliderFloat("Phi", &phi, 0.0, 6.28);
        ImGuiUtils::SliderFloat("Theta", &theta, 0.0, 0.5*3.14);
        ImGuiUtils::SliderFloat("Min Skylight", &m_MinSkylight, 0.0, 1.0);
        ImGuiUtils::ColorEdit3("Sun Color" , m_SunCol);
        ImGuiUtils::ColorEdit3("Sky Color" , m_SkyCol);
        ImGuiUtils::ColorEdit3("Refl Color", m_RefCol);
        ImGuiUtils::SliderFloat("Sun Strength" , &m_SunCol[3], 1.0, 5.0);
        ImGuiUtils::SliderFloat("Sky Strength" , &m_SkyCol[3], 1.0, 5.0);
        ImGuiUtils::SliderFloat("Refl Strength", &m_RefCol[3], 1.0, 5.0);
        ImGuiUtils::Checkbox("Materials", &m_Materials);
        ImGuiUtils::Checkbox("Fix Tiling", &m_FixTiling);
        ImGuiUtils::SliderFloat("Tiling Factor", &m_TilingFactor, 0.0, 64.0);
        ImGuiUtils::SliderFloat("Normal Strength", &m_NormalStrength, 0.0, 1.0);
        ImGui::End();
        
        if (phi != m_Phi || theta != m_Theta) {
            m_Phi = phi;
            m_Theta = theta;
            if (m_Shadows) m_Map.RequestShadowUpdate();
        }

        if (shadows != m_Shadows) {
            m_Shadows = shadows;
            if (m_Shadows) m_Map.RequestShadowUpdate();
        }

    }

    //-----Process updates:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //Update Maps
    m_Map.Update(m_Theta, m_Phi);

    //Return to normal rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
    
    if (m_Wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Renderer::OnWindowResize(unsigned int width, unsigned int height) {
    m_WindowWidth = width;
    m_WindowHeight = height;

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
