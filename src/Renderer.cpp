#include "Renderer.h"

#include "glad/glad.h"

#include "imgui.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::OnUpdate() {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::OnImGuiUpdate() {
    ImGui::Begin("WE");
    ImGui::ColorEdit3("ClearColor", m_ClearColor);
    ImGui::End();
}
