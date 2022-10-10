#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"

Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height),
      m_Shader("res/shaders/test.vert", "res/shaders/test.frag")
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_QuadData), m_QuadData, 
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_QuadIndices), m_QuadIndices,
            GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 
            (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
}

Renderer::~Renderer() {}

void Renderer::OnUpdate(float deltatime) {
    m_Camera.ProcessKeyboard(deltatime);

   float aspect = float(m_WindowWidth) / float(m_WindowHeight);

    glm::mat4 proj = glm::perspective(glm::radians(m_Camera.getFov()), aspect,
                                     0.1f, 100.0f);
    glm::mat4 view = m_Camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_MVP = proj * view * model;
}

void Renderer::OnRender() {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_Shader.Bind();
    m_Shader.setUniformMatrix4fv("uMVP", m_MVP);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::OnImGuiRender() {
    if (!m_ShowMenu) return;

    ImGui::Begin("WE");
    ImGui::ColorEdit3("ClearColor", m_ClearColor);
    ImGui::End();
}

void Renderer::OnWindowResize(unsigned int width, unsigned int height) {
    m_WindowWidth = width;
    m_WindowHeight = height;

    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Renderer::OnKeyPressed(int keycode, bool repeat) {
    if (keycode == LOFI_KEY_ESCAPE && !repeat)
        m_ShowMenu = (!m_ShowMenu);

    if (!repeat) {
        switch(keycode) {
            case LOFI_KEY_W: {
                m_Camera.Movement = m_Camera.Movement | CameraMovement::Forward;
                break;
            }
            case LOFI_KEY_S: {
                m_Camera.Movement = m_Camera.Movement | CameraMovement::Backward;
                break;
            }
            case LOFI_KEY_A: {
                m_Camera.Movement = m_Camera.Movement | CameraMovement::Left;
                break;
            }
            case LOFI_KEY_D: {
                m_Camera.Movement = m_Camera.Movement | CameraMovement::Right;
                break;
            }
        }
    }
}

void Renderer::OnKeyReleased(int keycode) {
    switch(keycode) {
        case LOFI_KEY_W: {
            m_Camera.Movement = m_Camera.Movement & ~CameraMovement::Forward;
            break;
        }
        case LOFI_KEY_S: {
            m_Camera.Movement = m_Camera.Movement & ~CameraMovement::Backward;
            break;
        }
        case LOFI_KEY_A: {
            m_Camera.Movement = m_Camera.Movement & ~CameraMovement::Left;
            break;
        }
        case LOFI_KEY_D: {
            m_Camera.Movement = m_Camera.Movement & ~CameraMovement::Right;
            break;
        }
    }
}

void Renderer::OnMouseMoved(float x, float y) {}
