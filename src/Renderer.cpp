#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"

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
        m_ShadedShader.setUniform3f("sun_col", m_SunCol);
        m_ShadedShader.setUniform3f("sky_col", m_SkyCol);
        m_ShadedShader.setUniform3f("ref_col", m_RefCol);
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
        ImGui::ColorEdit3("ClearColor", m_ClearColor);
        ImGui::End();
    }
    
    if (m_ShowCamMenu) {
        CameraSettings temp = m_Camera.getSettings();

        ImGui::Begin("Camera", &m_ShowCamMenu);
        ImGui::SliderFloat("Speed", &(temp.Speed), 0.0, 10.0f);
        ImGui::SliderFloat("Sensitivity", &(temp.Sensitivity), 0.0f, 200.0f);
        ImGui::SliderFloat("Fov", &(temp.Fov), 0.0f, 90.0f);
        ImGui::End();

        m_Camera.setSettings(temp);
    }
    
    if (m_ShowTerrainMenu)
        m_Map.ImGuiTerrain(m_ShowTerrainMenu, m_Shadows);

    if(m_ShowShadowMenu)
        m_Map.ImGuiShadowmap(m_ShowShadowMenu, m_Shadows);

    if(m_ShowMaterialMenu)
        m_Material.OnImGui();

    if (m_ShowLightMenu) {
        float phi = m_Phi, theta = m_Theta;
        bool shadows = m_Shadows, materials = m_Materials;

        ImGui::Begin("Lighting", &m_ShowLightMenu);
        ImGui::Checkbox("Shadows", &shadows);
        ImGui::SliderFloat("phi", &phi, 0.0, 6.28);
        ImGui::SliderFloat("theta", &theta, 0.0, 0.5*3.14);
        ImGui::ColorEdit3("SunColor" , glm::value_ptr(m_SunCol));
        ImGui::ColorEdit3("SkyColor" , glm::value_ptr(m_SkyCol));
        ImGui::ColorEdit3("ReflColor", glm::value_ptr(m_RefCol));
        ImGui::Checkbox("Materials", &materials);
        ImGui::SliderFloat("Tiling Factor", &m_TilingFactor, 0.0, 64.0);
        ImGui::SliderFloat("Normal Strength", &m_NormalStrength, 0.0, 1.0);
        ImGui::End();
        
        if (phi != m_Phi || theta != m_Theta || materials != m_Materials) {
            m_Phi = phi;
            m_Theta = theta;
            m_Materials = materials;
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
