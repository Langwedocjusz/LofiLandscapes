#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"

#include <iostream>


void Renderer::RenderHeightmap(bool normal_only) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    //Render to heightmap/normalmap:
    m_Map.Update(normal_only, m_Theta, m_Phi);

    //Update vertex data:
    m_Map.BindHeightmap();
    m_Terrain.DisplaceVertices(m_Map.getSettings().ScaleXZ, m_Map.getSettings().ScaleY,
                               m_Camera.getPos().x, m_Camera.getPos().z);

    //Return to normal rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
    
    if (m_Wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height),
      m_ShadedShader("res/shaders/shaded.vert", 
                     "res/shaders/shaded.frag"),
      m_WireframeShader("res/shaders/wireframe.vert", 
                        "res/shaders/wireframe.frag"),
      m_Terrain(),
      m_Map()
{
    glEnable(GL_DEPTH_TEST);
    RenderHeightmap(false);
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

    if (m_UpdatePos) {
        m_Map.BindHeightmap();
        m_Terrain.DisplaceVertices(m_Map.getSettings().ScaleXZ, m_Map.getSettings().ScaleY,
                                   m_Camera.getPos().x, m_Camera.getPos().z);
    }

    if (m_Wireframe) {
        m_WireframeShader.Bind();
        m_WireframeShader.setUniform1f("uL", m_Map.getSettings().ScaleXZ);
        m_WireframeShader.setUniform2f("uPos", m_Camera.getPos().x, m_Camera.getPos().z);
        m_WireframeShader.setUniformMatrix4fv("uMVP", m_MVP);

    }

    else {
        m_ShadedShader.Bind();
        m_ShadedShader.setUniform1f("uL", m_Map.getSettings().ScaleXZ);
        m_ShadedShader.setUniform1f("uTheta", m_Theta);
        m_ShadedShader.setUniform1f("uPhi", m_Phi);
        m_ShadedShader.setUniform2f("uPos", m_Camera.getPos().x, m_Camera.getPos().z);
        m_ShadedShader.setUniformMatrix4fv("uMVP", m_MVP);
        m_ShadedShader.setUniform1i("uShadow", int(m_Shadows));

        m_Map.BindNormalmap(0);
        m_ShadedShader.setUniform1i("normalmap", 0);
        m_Map.BindShadowmap(1);
        m_ShadedShader.setUniform1i("shadowmap", 1);
    }

    m_Terrain.BindGeometry();
    m_Terrain.Draw();
}

void Renderer::OnImGuiRender() {
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
            if (ImGui::MenuItem("Terrain")) {
                m_ShowTerrainMenu = !m_ShowTerrainMenu;
            }

            if (ImGui::MenuItem("Background")) {
                m_ShowBackgroundMenu = !m_ShowBackgroundMenu;
            }

            if (ImGui::MenuItem("Lighting")) {
                m_ShowLightMenu = !m_ShowLightMenu;
            }

            if (ImGui::MenuItem("Camera")) {
                m_ShowCamMenu = !m_ShowCamMenu;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (m_ShowBackgroundMenu) {
        ImGui::Begin("Background");
        ImGui::ColorEdit3("ClearColor", m_ClearColor);
        ImGui::End();
    }

    if (m_ShowTerrainMenu) {
        ImGui::Begin("Terrain");
        HeightmapSettings temp = m_Map.getSettings();
        ImGui::Checkbox("Update position", &m_UpdatePos);
        ImGui::SliderFloat("Scale xz", &(temp.ScaleXZ), 0.0f, 400.0f);
        ImGui::SliderFloat("Scale y" , &(temp.ScaleY ), 0.0f, 100.0f);
        ImGui::SliderInt("Octaves", &(temp.Octaves), 1, 16);
        ImGui::SliderFloat("Offset x", &(temp.Offset[0]), 0.0f, 20.0f);
        ImGui::SliderFloat("Offset y", &(temp.Offset[1]), 0.0f, 20.0f);

        if (temp != m_Map.getSettings()) {
            m_Map.setSettings(temp);
            RenderHeightmap(false);
        }

        else if (temp.ScaleY != m_Map.getSettings().ScaleY || temp.ScaleXZ != m_Map.getSettings().ScaleXZ) {
            m_Map.setSettings(temp);
            RenderHeightmap(true);
        }

        ImGui::End();
    }

    if (m_ShowLightMenu) {
        ImGui::Begin("Lighting");
        float phi = m_Phi, theta = m_Theta;
        ImGui::Checkbox("Shadows", &m_Shadows);
        ImGui::SliderFloat("phi", &phi, 0.0, 6.28);
        ImGui::SliderFloat("theta", &theta, 0.0, 0.5*3.14);
        if (phi != m_Phi || theta != m_Theta) {
            m_Phi = phi;
            m_Theta = theta;
            RenderHeightmap(true);
        }
        ImGui::End();
    }

    if (m_ShowCamMenu) {
        CameraSettings temp = m_Camera.getSettings();

        ImGui::Begin("Camera");
        ImGui::SliderFloat("Speed", &(temp.Speed), 0.0, 10.0f);
        ImGui::SliderFloat("Sensitivity", &(temp.Sensitivity), 0.0f, 200.0f);
        ImGui::SliderFloat("Fov", &(temp.Fov), 0.0f, 90.0f);
        ImGui::End();

        m_Camera.setSettings(temp);
    }
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
