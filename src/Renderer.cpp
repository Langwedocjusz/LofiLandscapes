#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"

Renderer::Renderer() 
    : m_Shader("res/shaders/test.vert", "res/shaders/test.frag")
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

void Renderer::OnUpdate() {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_Shader.Bind();
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::OnImGuiUpdate() {
    if (!m_ShowMenu) return;

    ImGui::Begin("WE");
    ImGui::ColorEdit3("ClearColor", m_ClearColor);
    ImGui::End();
}

void Renderer::OnWindowResize(unsigned int width, unsigned int height) {}

void Renderer::OnKeyPressed(int keycode, bool repeat) {
    if (keycode == LOFI_KEY_ESCAPE && !repeat)
        m_ShowMenu = (!m_ShowMenu);
}

void Renderer::OnKeyReleased(int keycode) {}
void Renderer::OnMouseMoved(float x, float y) {}
