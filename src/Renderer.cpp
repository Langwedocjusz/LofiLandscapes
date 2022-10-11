#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"

#include <iostream>

bool operator==(const HeightmapParams& lhs, const HeightmapParams& rhs) {
    return (lhs.Octaves == rhs.Octaves);
}

void Renderer::RenderHeightmap() {
    //Render to texture:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindTexture(GL_TEXTURE_2D, m_TargetTexture);
    glViewport(0, 0, m_HeightmapParams.Resolution, m_HeightmapParams.Resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_QuadShader.Bind();
    m_QuadShader.setUniform1i("uOctaves", m_HeightmapParams.Octaves);

    m_Scene.BindQuad();
    m_Scene.DrawQuad();

    //Return to normal rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
    
    if (m_Wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height),
      m_ShadedShader("res/shaders/shaded.vert", "res/shaders/shaded.frag"),
      m_WireframeShader("res/shaders/wireframe.vert", "res/shaders/wireframe.frag"),
      m_QuadShader("res/shaders/quad.vert", "res/shaders/quad.frag"),
      m_Scene(m_N, m_L)
{
    //Framebuffer
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    //Texture
    const unsigned int tex_res = 4096;
    glGenTextures(1, &m_TargetTexture);
    glBindTexture(GL_TEXTURE_2D, m_TargetTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_res, tex_res, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    //Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_TargetTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "FRAMEBUFFER NOT READY \n";

    glEnable(GL_DEPTH_TEST);

    RenderHeightmap();
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

    if (m_Wireframe) {
        m_WireframeShader.Bind();
        m_WireframeShader.setUniform1f("uL", m_L);
        m_WireframeShader.setUniformMatrix4fv("uMVP", m_MVP);

    }

    else {
        m_ShadedShader.Bind();
        m_ShadedShader.setUniform1f("uL", m_L);
        m_ShadedShader.setUniformMatrix4fv("uMVP", m_MVP);
    }

    glBindTexture(GL_TEXTURE_2D, m_TargetTexture);
    m_Scene.BindTerrain();
    m_Scene.DrawTerrain();
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
        int temp_octaves = m_HeightmapParams.Octaves;
        ImGui::SliderInt("Octaves", &temp_octaves, 1, 10);
    
        if (temp_octaves != m_HeightmapParams.Octaves) {
            m_HeightmapParams.Octaves = temp_octaves;
            RenderHeightmap();
        }

        ImGui::End();
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
